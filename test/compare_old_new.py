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
TOWERS_ETA_PER_CARD = 17
TOWERS_PHI_PER_CARD = 6

######## Utility functions

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

def convert_tower_global_iEta(iEta,n_cards=24):
    for cc in range(n_cards):
        these_iEta = iEta[:,cc,:]
        if (cc % 2 == 1):
            these_global_iEta = TOWERS_ETA_PER_CARD + these_iEta
        else:
            these_global_iEta = TOWERS_ETA_PER_CARD - 1 - these_iEta

        these_global_iEta_wrapped = ak.singletons(these_global_iEta,axis=0)
        if cc == 0:
            out = these_global_iEta_wrapped
        else:
            out = ak.concatenate((out,these_global_iEta_wrapped),axis=1)

    return out

def convert_tower_global_iPhi(iPhi,n_cards=24):
    for cc in range(n_cards):
        these_iPhi = iPhi[:,cc,:]
        if (cc % 2 == 1):
            these_global_iPhi = ((cc // 2) * TOWERS_PHI_PER_CARD) + these_iPhi
        else:
            these_global_iPhi = ((cc // 2) * TOWERS_PHI_PER_CARD) + (TOWERS_PHI_PER_CARD - 1 - these_iPhi)

        these_global_iPhi_wrapped = ak.singletons(these_global_iPhi,axis=0)
        if cc == 0:
            out = these_global_iPhi_wrapped
        else:
            out = ak.concatenate((out,these_global_iPhi_wrapped),axis=1)

    return out

def get_new_tree():
    with uproot.open(NEW_PATH) as f:
        tree = f["l1NtupleProducer/linkTree;1"]
    return tree

def get_old_tree():
    with uproot.open(OLD_PATH) as f:
        tree = f["l1NtupleOldProducer/outputTree;1"]
    return tree

######### Cluster functions

def top2_clusters(pt,eta,phi):
    sorted_inds = ak.argsort(pt,ascending=False)
    top2_inds = sorted_inds[:,:2]
    return pt[top2_inds], eta[top2_inds], phi[top2_inds]

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

######## Tower functions

def compare_towers(old_tree,new_tree):
    new_et = new_tree["tower_et"].array()*0.5 #Account for LSB
    new_eta = new_tree["tower_eta"].array()
    new_phi = new_tree["tower_phi"].array()
    new_hoe = new_tree["tower_hoe"].array()
    old_ecal_et = old_tree["tower_ecal_et"].array()
    old_hcal_et = old_tree["tower_hcal_et"].array()
    old_eta = old_tree["tower_eta"].array()
    old_phi = old_tree["tower_phi"].array()
    old_et = old_ecal_et + old_hcal_et
    old_hoe = old_hcal_et/old_et

    #Convert new emulator coordinates from relative to card to global
    new_eta = convert_tower_global_iEta(new_eta)
    new_phi = convert_tower_global_iPhi(new_phi)

    #Flatten new emulator towers to look at all cards at once
    new_et = ak.flatten(new_et,axis=2)
    new_eta = ak.flatten(new_eta,axis=2)
    new_phi = ak.flatten(new_phi,axis=2)
    new_hoe = ak.flatten(new_hoe,axis=2)

    #Match towers by location between old and new emulator
    eta_pairs = ak.unzip(ak.cartesian((old_eta,new_eta))) #eta_pairs[0] is old
    phi_pairs = ak.unzip(ak.cartesian((old_phi,new_phi)))
    #Require old and new emulator agree on eta and phi
    filt = ((eta_pairs[0] == eta_pairs[1]) & (phi_pairs[0] == phi_pairs[1]))
    et_pairs = ak.unzip(ak.cartesian((old_et,new_et)))
    hoe_pairs = ak.unzip(ak.cartesian((old_hoe,new_hoe)))
    all_et_diffs = (et_pairs[1] - et_pairs[0])/et_pairs[0]
    all_hoe_diffs = (hoe_pairs[1] - hoe_pairs[0])
    #Only consider diffs when old and new tower have identical eta and phi
    et_diffs = all_et_diffs[filt]
    hoe_diffs = all_hoe_diffs[filt]

    return et_diffs, hoe_diffs

def get_new_et_hoe(new_tree):
    et = new_tree["tower_et"].array()
    hoe = new_tree["tower_hoe"].array()
    return ak.ravel(et), ak.ravel(hoe)

def get_old_et_hoe(old_tree):
    ecal_et = old_tree["tower_ecal_et"].array()
    hcal_et = old_tree["tower_hcal_et"].array()
    et = ecal_et + hcal_et
    hoe = hcal_et/et
    return ak.ravel(et), ak.ravel(hoe)

######## Plotting and testing

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

############## Main

def main():
    new_tree = get_new_tree()
    old_tree = get_old_tree()

    pt_diff, eta_diff, phi_diff = compare_clusters(old_tree,new_tree)
    et_diff, hoe_diff = compare_towers(old_tree,new_tree)

    filt = phi_diff > -1

    pt_diff_axis = axis.Regular(20,-1,1,name="pt_diff",label=r"$p_t$ Difference [GeV]")
    phi_diff_axis = axis.Regular(32,0,3.2,name="phi_diff",label=r"$\phi$ Difference")
    eta_diff_axis = axis.Regular(60,-3,3,name="eta_diff",label=r"$\eta$ Difference")
    hoe_diff_axis = axis.Regular(44,-1.0,1.2,name="hoe_diff",label="HOE Difference")

    pt_axis = axis.Regular(20,0,100,name="pt",label="pt")
    et_axis = axis.Regular(40,0,20,name="et",label="et")
    phi_axis = axis.Regular(32,-3.2,3.2,name="phi",label="phi")
    eta_axis = axis.Regular(15,-1.5,1.5,name="eta",label="eta")
    hoe_axis = axis.Regular(24,0.0,1.2,name="hoe",label="hoe")

    #Cluster difference histograms
    pt_hist = Hist(pt_diff_axis,name="Events")
    eta_hist = Hist(eta_diff_axis,name="Events")
    phi_hist = Hist(phi_diff_axis,name="Events")

    #Cluster histograms
    new_pt_hist = Hist(pt_axis,name="Events")
    old_pt_hist = Hist(pt_axis,name="Events")
    new_phi_hist = Hist(phi_axis,name="Events")
    old_phi_hist = Hist(phi_axis,name="Events")
    new_eta_hist = Hist(eta_axis,name="Events")
    old_eta_hist = Hist(eta_axis,name="Events")

    #Tower difference histograms
    tower_et_hist = Hist(pt_diff_axis,name="Events")
    tower_hoe_hist = Hist(hoe_diff_axis,name="Events")

    #Tower histograms
    new_tower_et_hist = Hist(et_axis,name="Events")
    old_tower_et_hist = Hist(et_axis,name="Events")
    new_tower_hoe_hist = Hist(hoe_axis,name="Events")
    old_tower_hoe_hist = Hist(hoe_axis,name="Events")

    pt_hist.fill(pt_diff=ak.ravel(pt_diff[filt]))
    eta_hist.fill(eta_diff=ak.ravel(eta_diff[filt]))
    phi_hist.fill(phi_diff=ak.ravel(phi_diff[filt]))

    tower_et_hist.fill(pt_diff=ak.ravel(et_diff))
    tower_hoe_hist.fill(hoe_diff=ak.ravel(hoe_diff))

    new_pt, new_eta, new_phi = get_new_top2_clusters(new_tree)
    old_pt, old_eta, old_phi = get_old_top2_clusters(old_tree)

    new_pt_hist.fill(ak.ravel(new_pt[filt]))
    new_phi_hist.fill(ak.ravel(new_phi[filt]))
    new_eta_hist.fill(ak.ravel(new_eta[filt]))
    old_pt_hist.fill(ak.ravel(old_pt[filt]))
    old_phi_hist.fill(ak.ravel(old_phi[filt]))
    old_eta_hist.fill(ak.ravel(old_eta[filt]))

    new_et, new_hoe = get_new_et_hoe(new_tree)
    old_et, old_hoe = get_old_et_hoe(old_tree)

    new_tower_et_hist.fill(new_et)
    new_tower_hoe_hist.fill(new_hoe)
    old_tower_et_hist.fill(old_et)
    old_tower_hoe_hist.fill(old_hoe)

    Path("plots").mkdir(parents=True,exist_ok=True)

    plot(pt_hist,"plots/pt_diff.png",r"Cluster $p_t$ Relative Difference",xlabel=r"$p_t$ Rel. Diff. [GeV]")
    plot(eta_hist,"plots/eta_diff.png",r"Cluster $\eta$ Difference",xlabel=r"$\eta$ Diff.")
    plot(phi_hist,"plots/phi_diff.png",r"Cluster$\phi$ Difference",xlabel=r"$\phi$ Diff.")

    plot(new_pt_hist,"plots/new_pt.png",r"New $p_t$",xlabel=r"$p_t$ [GeV]")
    plot(old_pt_hist,"plots/old_pt.png",r"Old $p_t$",xlabel=r"$p_t$ [GeV]")
    plot(new_eta_hist,"plots/new_eta.png",r"New $\eta$",xlabel=r"$\eta$")
    plot(old_eta_hist,"plots/old_eta.png",r"Old $\eta$",xlabel=r"$\eta$")
    plot(new_phi_hist,"plots/new_phi.png",r"New $\phi$",xlabel=r"$\phi$")
    plot(old_phi_hist,"plots/old_phi.png",r"Old $\phi$",xlabel=r"$\phi$")

    plot(tower_et_hist,"plots/tower_et_diff.png",r"Tower $E_T$ Relative Difference",xlabel=r"$E_T$ Rel. Diff. [GeV]")
    plot(tower_hoe_hist,"plots/tower_hoe_diff.png",r"Tower HOE Difference",xlabel="HOE Diff.")
    
    plot(new_tower_et_hist,"plots/new_tower_et.png",r"New $E_T$",xlabel=r"$E_T$ [GeV]")
    plot(old_tower_et_hist,"plots/old_tower_et.png",r"Old $E_T$",xlabel=r"$E_T$ [GeV]")
    plot(new_tower_hoe_hist,"plots/new_tower_hoe.png","New HOE",xlabel="New HOE")
    plot(old_tower_hoe_hist,"plots/old_tower_hoe.png","Old HOE","Old HOE")

if __name__ == "__main__":
    main()