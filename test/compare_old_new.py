import awkward as ak
import matplotlib.pyplot as plt
from hist import axis, Hist
import hist.plot
import mplhep as hep
import numpy as np
from pathlib import Path
import uproot

NEW_PATH = "analyzer.root"
OLD_PATH = "old_analyzer.root"

ECAL_ETA_RANGE = 1.4841
HCAL_ETA_RANGE = 1.3968
CRYSTAL_SIZE_ETA = ECAL_ETA_RANGE/85 #85 crystals per card
CRYSTAL_SIZE_PHI = np.pi / 180 #12 cards cover 2*pi, 30 crystals per card
PHI_CARD_LENGTH = np.pi / 6

def convert_to_eta(iEta,n_cards=24):
    for cc in range(n_cards):
        sign = 2*(cc % 2) - 1 #if cc odd, positive eta

        these_iEta = iEta[:,cc,:]
        these_eta = sign*CRYSTAL_SIZE_ETA*(0.5 + these_iEta)

        these_eta_wrapped = ak.singletons(these_eta,axis=0)
        if cc == 0:
            out = these_eta_wrapped
        else:
            out = ak.concatenate((out,these_eta_wrapped),axis=1)

    return out

def convert_to_phi(iPhi,n_cards=24):
    for cc in range(n_cards):
        #if cc even, iPhi 0 is further in phi
        sign = 2*(cc % 2) - 1
        if cc % 2 == 0:
            offset = (cc//2 + 1) * PHI_CARD_LENGTH
        else:
            offset = cc//2 * PHI_CARD_LENGTH

        these_iPhi = iPhi[:,cc,:]
        these_phi = offset + sign*CRYSTAL_SIZE_PHI*(0.5 + these_iPhi)

        these_phi_wrapped = ak.singletons(these_phi,axis=0)
        if cc == 0:
            out = these_phi_wrapped
        else:
            out = ak.concatenate((out,these_phi_wrapped),axis=1)
    
    #Shift values > pi to be negative
    return ak.where(out < np.pi, out, out - 2*np.pi)

def top2_clusters(pt,eta,phi):
    sorted_inds = ak.argsort(pt,ascending=False)
    top2_inds = sorted_inds[:,:2]
    return pt[top2_inds], eta[top2_inds], phi[top2_inds]

def get_new_tree():
    with uproot.open(NEW_PATH) as f:
        tree = f["l1NtupleProducer/linkTree;1"]
    return tree

def get_old_tree():
    with uproot.open(OLD_PATH) as f:
        tree = f["l1NtupleOldProducer/outputTree;1"]
    return tree

def compare_clusters(old_tree,new_tree):
    new_pt = new_tree["cluster_pt"].array()*0.5 #account for LSB
    new_eta = new_tree["cluster_eta"].array()
    new_phi = new_tree["cluster_phi"].array()
    old_pt = old_tree["cluster_pt"].array()
    old_eta = old_tree["cluster_eta"].array()
    old_phi = old_tree["cluster_phi"].array()

    #Convert new emulator coordinates from relative to card to global
    new_eta = convert_to_eta(new_eta)
    new_phi = convert_to_phi(new_phi)

    #Flatten new emulator clusters to look at all cards at once
    new_pt = ak.flatten(new_pt,axis=2)
    new_eta = ak.flatten(new_eta,axis=2)
    new_phi = ak.flatten(new_phi,axis=2)

    #Get top 2 clusters
    new_pt, new_eta, new_phi = top2_clusters(new_pt,new_eta,new_phi)
    old_pt, old_eta, old_phi = top2_clusters(old_pt,old_eta,old_phi)

    pt_diff = new_pt - old_pt
    eta_diff = new_eta - old_eta
    phi_diff = abs(new_phi - old_phi)

    return pt_diff/old_pt, eta_diff, ak.where(phi_diff < np.pi, phi_diff, 2*np.pi - phi_diff)

def get_new_top2_clusters(tree):
    new_pt = tree["cluster_pt"].array()*0.5 #account for LSB
    new_eta = tree["cluster_eta"].array()
    new_phi = tree["cluster_phi"].array()

    #Convert new emulator coordinates from relative to card to global
    new_eta = convert_to_eta(new_eta)
    new_phi = convert_to_phi(new_phi)

    #Flatten new emulator clusters to look at all cards at once
    new_pt = ak.flatten(new_pt,axis=2)
    new_eta = ak.flatten(new_eta,axis=2)
    new_phi = ak.flatten(new_phi,axis=2)

    #Get top 2 clusters
    return top2_clusters(new_pt,new_eta,new_phi)

def get_old_top2_clusters(tree):
    old_pt = tree["cluster_pt"].array()
    old_eta = tree["cluster_eta"].array()
    old_phi = tree["cluster_phi"].array()

    return top2_clusters(old_pt,old_eta,old_phi)

def plot(h,outfile,title=None,xlabel=None,logy=False,flow="hint"):
    fig, ax = plt.subplots(1,1,figsize=(7,4.7))
    
    hep.histplot(h,ax=ax,flow=flow,yerr=False)

    if title is not None:
        ax.set_title(title,pad=2)
    ax.set_xlabel(xlabel,fontsize=10,labelpad=2)
    if logy:
        ax.semilogy()
    plt.savefig(outfile)
    print(f"Saved output to {outfile}")

def test_convert_to_eta():
    test_in = ak.Array([[[1,2,3],[4,5,16]]])
    print(test_in.show())
    test_out = convert_to_eta(test_in,n_cards=2)
    print("-----------------------")
    print(test_out.show())

def test_convert_to_phi():
    test_in = ak.Array([[[1,2,3],[4,5,16]]])
    print(test_in.show())
    test_out = convert_to_phi(test_in,n_cards=2)
    print("-----------------------")
    print(test_out.show())

def main():
    new_tree = get_new_tree()
    old_tree = get_old_tree()

    pt_diff, eta_diff, phi_diff = compare_clusters(old_tree,new_tree)

    filt = phi_diff > -1

    pt_diff_axis = axis.Regular(20,-1,1,name="pt_diff",label=r"$p_t$ Difference [GeV]")
    phi_diff_axis = axis.Regular(32,0,3.2,name="phi_diff",label=r"$\phi$ Difference")
    eta_diff_axis = axis.Regular(60,-3,3,name="eta_diff",label=r"$\eta$ Difference")

    pt_axis = axis.Regular(20,0,100,name="pt",label="pt")
    phi_axis = axis.Regular(32,-3.2,3.2,name="phi",label="phi")
    eta_axis = axis.Regular(15,-1.5,1.5,name="eta",label="eta")

    pt_hist = Hist(pt_diff_axis,name="Events")
    eta_hist = Hist(eta_diff_axis,name="Events")
    phi_hist = Hist(phi_diff_axis,name="Events")

    new_pt_hist = Hist(pt_axis,name="Events")
    old_pt_hist = Hist(pt_axis,name="Events")
    new_phi_hist = Hist(phi_axis,name="Events")
    old_phi_hist = Hist(phi_axis,name="Events")
    new_eta_hist = Hist(eta_axis,name="Events")
    old_eta_hist = Hist(eta_axis,name="Events")

    pt_hist.fill(pt_diff=ak.ravel(pt_diff[filt]))
    eta_hist.fill(eta_diff=ak.ravel(eta_diff[filt]))
    phi_hist.fill(phi_diff=ak.ravel(phi_diff[filt]))

    new_pt, new_eta, new_phi = get_new_top2_clusters(new_tree)
    old_pt, old_eta, old_phi = get_old_top2_clusters(old_tree)

    new_pt_hist.fill(ak.ravel(new_pt[filt]))
    new_phi_hist.fill(ak.ravel(new_phi[filt]))
    new_eta_hist.fill(ak.ravel(new_eta[filt]))
    old_pt_hist.fill(ak.ravel(old_pt[filt]))
    old_phi_hist.fill(ak.ravel(old_phi[filt]))
    old_eta_hist.fill(ak.ravel(old_eta[filt]))

    Path("plots").mkdir(parents=True,exist_ok=True)

    plot(pt_hist,"plots/pt_diff.png",r"$p_t$ Relative Difference",xlabel=r"$p_t$ Rel. Diff. [GeV]")
    plot(eta_hist,"plots/eta_diff.png",r"$\eta$ Difference",xlabel=r"$\eta$ Diff.")
    plot(phi_hist,"plots/phi_diff.png",r"$\phi$ Difference",xlabel=r"$\phi$ Diff.")

    plot(new_pt_hist,"plots/new_pt.png",r"New $p_t$",xlabel=r"$p_t$ [GeV]")
    plot(old_pt_hist,"plots/old_pt.png",r"Old $p_t$",xlabel=r"$p_t$ [GeV]")
    plot(new_eta_hist,"plots/new_eta.png",r"New $\eta$",xlabel=r"$\eta$")
    plot(old_eta_hist,"plots/old_eta.png",r"Old $\eta$",xlabel=r"$\eta$")
    plot(new_phi_hist,"plots/new_phi.png",r"New $\phi$",xlabel=r"$\phi$")
    plot(old_phi_hist,"plots/old_phi.png",r"Old $\phi$",xlabel=r"$\phi$")

if __name__ == "__main__":
    main()