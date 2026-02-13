import awkward as ak
import matplotlib.pyplot as plt
from hist import axis, Hist
import mplhep as hep
import numpy as np
import uproot

EVENT_TO_PLOT = 0 #Decides which event to make event displays of when running
CARD_TO_PLOT = 0 #Decides which RCT card's information to make displays of

FEATURE_PATH = "MLonRCT_features.root"
PLOT_DIR = "plots_MLonRCT/"

ECAL_ETA_RANGE = 1.4841
HCAL_ETA_RANGE = 1.3968
CRYSTAL_SIZE_ETA = ECAL_ETA_RANGE/85 #85 crystals per card
CRYSTAL_SIZE_PHI = np.pi / 180 #12 cards cover 2*pi, 30 crystals per card
TOWER_SIZE_ETA = ECAL_ETA_RANGE/17
TOWER_SIZE_PHI = np.pi / 36
PHI_CARD_LENGTH = np.pi / 6
ECAL_TOWERS_ETA_PER_CARD = 17
HCAL_TOWERS_ETA_PER_CARD = 16
TOWERS_PHI_PER_CARD = 6

######## Utility functions

def get_feature_tree():
    '''
    Get the ROOT tree from the file produced by the MLonRCTSetupAnalyzer.
    '''
    with uproot.open(FEATURE_PATH) as f:
        tree = f["l1NtupleProducer/linkTree;1"]
    return tree

def get_clusters(tree,SLR):
    seed_energy = tree[f"SLR{SLR}_cluster_seed_energy"].array()*0.5 #account for LSB
    energy = tree[f"SLR{SLR}_cluster_energy"].array()*0.5
    eta = tree[f"SLR{SLR}_cluster_eta"].array()
    phi = tree[f"SLR{SLR}_cluster_phi"].array()
    et5x5 = tree[f"SLR{SLR}_cluster_et5x5"].array()*0.5
    et2x5 = tree[f"SLR{SLR}_cluster_et2x5"].array()*0.5
    timing = tree[f"SLR{SLR}_cluster_timing"].array()
    spike = tree[f"SLR{SLR}_cluster_spike"].array()
    satur = tree[f"SLR{SLR}_cluster_satur"].array()
    brems = tree[f"SLR{SLR}_cluster_brems"].array()
    spare = tree[f"SLR{SLR}_cluster_spare"].array()

    out = {
        "seed_energy": seed_energy,
        "energy": energy,
        "eta": eta,
        "phi": phi,
        "et5x5": et5x5,
        "et2x5": et2x5,
        "timing": timing,
        "spike": spike,
        "satur": satur,
        "brems": brems,
        "spare": spare
    }

    return out

def get_ecal_towers(tree,SLR):
    et = tree[f"ECALUnclusteredSLR{SLR}_tower_et"].array()*0.5
    eta = tree[f"ECALUnclusteredSLR{SLR}_tower_eta"].array()
    phi = tree[f"ECALUnclusteredSLR{SLR}_tower_phi"].array()
    timing = tree[f"ECALUnclusteredSLR{SLR}_tower_timing"].array()
    spike = tree[f"ECALUnclusteredSLR{SLR}_tower_spike"].array()

    out = {
        "et": et,
        "eta": eta,
        "phi": phi,
        "timing": timing,
        "spike": spike
    }

    return out

def get_hcal_towers(tree,link):
    et = tree[f"HCAL{link}_tower_et"].array()*0.5
    eta = tree[f"HCAL{link}_tower_eta"].array()
    phi = tree[f"HCAL{link}_tower_phi"].array()
    fb = tree[f"HCAL{link}_tower_fb"].array()

    out = {
        "et": et,
        "eta": eta,
        "phi": phi,
        "fb": fb
    }

    return out

######## Plotting functions

def plot_clusters(tree,iEvent=0,cc=0):
    cluster_info = {}
    for i in range(4):
        cluster_info[i] = get_clusters(tree,i)
    et = cluster_info[3]["energy"][iEvent][cc]
    iEta = cluster_info[3]["eta"][iEvent][cc]
    iPhi = cluster_info[3]["phi"][iEvent][cc]
    for i in range(3):
        et = ak.concatenate((et,cluster_info[i]["energy"][iEvent][cc]),axis=0)
        iEta = ak.concatenate((iEta,cluster_info[i]["eta"][iEvent][cc]),axis=0)
        iPhi = ak.concatenate((iPhi,cluster_info[i]["phi"][iEvent][cc]),axis=0)

    eta_axis = axis.Regular(ECAL_TOWERS_ETA_PER_CARD*5,-0.5,ECAL_TOWERS_ETA_PER_CARD*5-0.5,name="eta",label=r"$\eta$")
    phi_axis = axis.Regular(TOWERS_PHI_PER_CARD*5,-0.5,TOWERS_PHI_PER_CARD*5-0.5,name="phi",label=r"$\phi$")
    display_hist = Hist(eta_axis,phi_axis,name="Events")

    display_hist.fill(eta=ak.ravel(iEta),phi=ak.ravel(iPhi),weight=ak.ravel(et))

    fig,ax = plt.subplots(1,1,figsize=(7,4.7))
    hep.hist2dplot(display_hist,ax=ax,cmin=0.0)
    title = f"Clusters (Event {iEvent}, Card {cc})"
    ax.set_title(title,y=1.07,pad=2)
    ax.set_xlabel(r"$\eta$",fontsize=10)
    ax.set_ylabel(r"$\phi$",fontsize=10,labelpad=2)

    outfile = f"{PLOT_DIR}clusters_iEv{iEvent}_cc{cc}.png"
    plt.savefig(outfile,bbox_inches='tight')
    print(f"Saved output to {outfile}")

def plot_ecal_towers(tree,iEvent=0,cc=0):
    ecal_tower_info = {}
    for i in range(4):
        ecal_tower_info[i] = get_ecal_towers(tree,i)
    et = ecal_tower_info[3]["et"][iEvent][cc]
    iEta = ecal_tower_info[3]["eta"][iEvent][cc]
    iPhi = ecal_tower_info[3]["phi"][iEvent][cc]
    for i in range(3):
        et = ak.concatenate((et,ecal_tower_info[i]["et"][iEvent][cc]),axis=0)
        iEta = ak.concatenate((iEta,ecal_tower_info[i]["eta"][iEvent][cc]),axis=0)
        iPhi = ak.concatenate((iPhi,ecal_tower_info[i]["phi"][iEvent][cc]),axis=0)

    eta_axis = axis.Regular(ECAL_TOWERS_ETA_PER_CARD,-0.5,ECAL_TOWERS_ETA_PER_CARD-0.5,name="eta",label=r"$\eta$")
    phi_axis = axis.Regular(TOWERS_PHI_PER_CARD,-0.5,TOWERS_PHI_PER_CARD-0.5,name="phi",label=r"$\phi$")
    display_hist = Hist(eta_axis,phi_axis,name="Events")

    display_hist.fill(eta=ak.ravel(iEta),phi=ak.ravel(iPhi),weight=ak.ravel(et))

    fig,ax = plt.subplots(1,1,figsize=(7,4.7))
    hep.hist2dplot(display_hist,ax=ax,cmin=0.0)
    title = f"ECAL Towers (Event {iEvent}, Card {cc})"
    ax.set_title(title,y=1.07,pad=2)
    ax.set_xlabel(r"$\eta$",fontsize=10)
    ax.set_ylabel(r"$\phi$",fontsize=10,labelpad=2)

    outfile = f"{PLOT_DIR}ecal_towers_iEv{iEvent}_cc{cc}.png"
    plt.savefig(outfile,bbox_inches='tight')
    print(f"Saved output to {outfile}")

def plot_hcal_towers(tree,iEvent=0,cc=0):
    hcal_tower_info = {}
    for i in range(4):
        hcal_tower_info[i] = get_hcal_towers(tree,i+5)
    et = hcal_tower_info[3]["et"][iEvent][cc]
    iEta = hcal_tower_info[3]["eta"][iEvent][cc]
    iPhi = hcal_tower_info[3]["phi"][iEvent][cc]
    for i in range(3):
        et = ak.concatenate((et,hcal_tower_info[i]["et"][iEvent][cc]),axis=0)
        iEta = ak.concatenate((iEta,hcal_tower_info[i]["eta"][iEvent][cc]),axis=0)
        iPhi = ak.concatenate((iPhi,hcal_tower_info[i]["phi"][iEvent][cc]),axis=0)

    eta_axis = axis.Regular(HCAL_TOWERS_ETA_PER_CARD,-0.5,HCAL_TOWERS_ETA_PER_CARD-0.5,name="eta",label=r"$\eta$")
    phi_axis = axis.Regular(TOWERS_PHI_PER_CARD,-0.5,TOWERS_PHI_PER_CARD-0.5,name="phi",label=r"$\phi$")
    display_hist = Hist(eta_axis,phi_axis,name="Events")

    display_hist.fill(eta=ak.ravel(iEta),phi=ak.ravel(iPhi),weight=ak.ravel(et))

    fig,ax = plt.subplots(1,1,figsize=(7,4.7))
    hep.hist2dplot(display_hist,ax=ax,cmin=0.0)
    title = f"HCAL Towers (Event {iEvent}, Card {cc})"
    ax.set_title(title,y=1.07,pad=2)
    ax.set_xlabel(r"$\eta$",fontsize=10)
    ax.set_ylabel(r"$\phi$",fontsize=10,labelpad=2)

    outfile = f"{PLOT_DIR}hcal_towers_iEv{iEvent}_cc{cc}.png"
    plt.savefig(outfile,bbox_inches='tight')
    print(f"Saved output to {outfile}")

############## Main

def main():
    tree = get_feature_tree()

    plot_clusters(tree, EVENT_TO_PLOT, CARD_TO_PLOT)
    plot_ecal_towers(tree, EVENT_TO_PLOT, CARD_TO_PLOT)
    plot_hcal_towers(tree, EVENT_TO_PLOT, CARD_TO_PLOT)

if __name__ == "__main__":
    main()