#include "CombineHarvester/HTTSM2016/interface/HttSystematics_SMRun2.h"
#include <vector>
#include <string>
#include "CombineHarvester/CombineTools/interface/Systematics.h"
#include "CombineHarvester/CombineTools/interface/Process.h"
#include "CombineHarvester/CombineTools/interface/Utilities.h"

namespace ch {

using ch::syst::SystMap;
using ch::syst::SystMapAsymm;
using ch::syst::era;
using ch::syst::channel;
using ch::syst::bin_id;
using ch::syst::process;
using ch::syst::bin;
using ch::JoinStr;

void AddSMRun2Systematics(CombineHarvester & cb, int control_region, bool zmm_fit) {
  // Create a CombineHarvester clone that only contains the signal
  // categories
  CombineHarvester cb_sig = cb.cp();

  std::vector<std::string> ggH = {"ggH"};
  std::vector<std::string> qqH = {"qqH"};

  if (control_region == 1){
    // we only want to cosider systematic uncertainties in the signal region.
    // limit to only the 0jet/1jet and vbf categories
    cb_sig.bin_id({1,2,3,4,5,6,10,11,12,13,14,15});
  }

  auto signal = Set2Vec(cb.cp().signals().SetFromProcs(
      std::mem_fn(&Process::process)));

  signal = JoinStr({signal});


  // Electron and muon efficiencies
  // ------------------------------
  cb.cp().AddSyst(cb, "CMS_eff_m", "lnN", SystMap<channel, process>::init
    ({"zmm"}, {"ZTT", "TT", "VV", "ZL"},  1.04)
    ({"zmm"}, {"ZJ"},  1.02)
    ({"mt"}, JoinStr({signal, {"ZTT", "ZL", "ZJ"}}),  1.02)
    ({"em"}, JoinStr({signal, {"ZTT", "TT", "VV", "ZLL"}}),       1.02));
  
  cb.cp().AddSyst(cb, "CMS_eff_e", "lnN", SystMap<channel, process>::init
    ({"et"}, JoinStr({signal, {"ZTT",  "ZL", "ZJ"}}),  1.02)
    ({"em"}, JoinStr({signal, {"ZTT", "TT", "VV", "ZLL"}}),       1.02));




  // Tau-related systematics
  // -----------------------
  cb.cp().process(JoinStr({signal, {"ZTT"}})).channel({"et","mt","tt"}).AddSyst(cb,
    "CMS_eff_t_$ERA", "lnN", SystMap<channel>::init
    ({"et", "mt"}, 1.05)
    ({"tt"},       1.10));

  cb.cp().process(JoinStr({signal, {"ZTT"}})).channel({"et","mt","tt"}).AddSyst(cb,
    "CMS_eff_t_$CHANNEL_$ERA", "lnN", SystMap<channel>::init
    ({"et", "mt"}, 1.03)
    ({"tt"},       1.092));


  // Electron energy scale
  // ---------------------
  cb.cp().process(JoinStr({signal, {"ZTT"}})).channel({"em"}).AddSyst(cb,
    "CMS_scale_e_$CHANNEL_$ERA", "shape", SystMap<>::init(1.00));


  // Recoil corrections
  // ------------------
  // These should not be applied to the W in all control regions becasuse we should
  // treat it as an uncertainty on the low/high mT factor.
  // For now we also avoid applying this to any of the high-mT control regions
  // as the exact (anti-)correlation with low mT needs to be established
  // CHECK THIS
  cb.cp().AddSyst(cb,
    "CMS_htt_boson_scale_met_$ERA", "lnN", SystMap<channel, bin_id, process>::init
    ({"et", "mt", "em", "tt"}, {1, 2, 3, 4,5,6}, JoinStr({signal, {"ZTT", "W"}}), 1.02));



  // Cross-sections and lumi
  // -----------------------
  cb.cp().process(JoinStr({signal, {"TT","VV", "ZL", "ZJ", "ZTT", "ZLL"}})).AddSyst(cb,
    "lumi_13TeV", "lnN", SystMap<>::init(1.062));

  //Add luminosity uncertainty for W in em, tt and the zmm region as norm is from MC
  cb.cp().process({"W"}).channel({"tt","em","zmm"}).AddSyst(cb,
    "lumi_13TeV", "lnN", SystMap<>::init(1.062));

  if(zmm_fit)
  {
    // Add Z crosssection uncertainty on ZL, ZJ and ZLL (not ZTT due to taking into account the zmm control region).
    // Also don't add it for the zmm control region
    cb.SetFlag("filters-use-regex", true);
    cb.cp().channel({"zmm"},false).process({"ZL", "ZJ", "ZLL"}).AddSyst(cb,
        "CMS_htt_zjXsec_13TeV", "lnN", SystMap<>::init(1.04));
    cb.cp().channel({"zmm"},false).bin({"_cr$"}).process({"ZTT"}).AddSyst(cb,
        "CMS_htt_zjXsec_13TeV", "lnN", SystMap<>::init(1.04));
    cb.SetFlag("filters-use-regex", false);

    cb.FilterSysts([](ch::Systematic *syst) {
      return syst->name() == "lumi_13TeV" &&
        (
          (syst->channel() == "zmm" && syst->process() == "ZL") ||
          (syst->channel() != "zmm" && syst->process() == "ZTT" &&
            (syst->bin_id() == 8 || syst->bin_id() == 9))
        );
    });
  }
  else
  {
    cb.cp().process({"ZTT", "ZL", "ZJ", "ZLL"}).AddSyst(cb,
        "CMS_htt_zjXsec_13TeV", "lnN", SystMap<>::init(1.04));
  }

//   Diboson and ttbar Normalisation - fully correlated
  cb.cp().process({"VV"}).AddSyst(cb,
    "CMS_htt_vvXsec_13TeV", "lnN", SystMap<>::init(1.05));

  cb.cp().process({"TT"}).AddSyst(cb,
    "CMS_htt_tjXsec_13TeV", "lnN", SystMap<>::init(1.06));

  // W norm, just for em, tt and the zmm region where MC norm is from MC
  cb.cp().process({"W"}).channel({"tt","em","zmm"}).AddSyst(cb,
    "CMS_htt_wjXsec_13TeV", "lnN", SystMap<>::init(1.04));


  // Fake-rates
  // ----------
  cb.cp().process({"ZL"}).channel({"et"}).AddSyst(cb,
    "CMS_htt_eFakeTau_tight_13TeV", "lnN", SystMap<>::init(1.30));

  cb.cp().process({"ZL"}).channel({"tt"}).AddSyst(cb,
    "CMS_htt_eFakeTau_loose_13TeV", "lnN", SystMap<>::init(1.10));
  
  cb.cp().process({"W"}).channel({"tt"}).AddSyst(cb,
    "CMS_htt_jetFakeTau_$CHANNEL_13TeV", "lnN", SystMap<>::init(1.20));

  cb.cp().process({"ZL"}).channel({"mt"}).AddSyst(cb,
    "CMS_htt_mFakeTau_tight_13TeV", "lnN", SystMap<>::init(1.30)); //Uncertainty ranges from 7-35% depending on eta bin

  cb.cp().process({"ZL"}).channel({"tt"}).AddSyst(cb,
    "CMS_htt_mFakeTau_loose_13TeV", "lnN", SystMap<>::init(1.20)); //Uncertainty ranges from 5-23% depending on eta bin






//  if (control_region == 0) {
//    // the uncertainty model in the signal region is the classical one

//    }
    
    
    if (control_region == 1) {
      // Create rateParams for control regions:
      //  - [x] 1 rateParam for all W in every region
      //  - [x] 1 rateParam for QCD in low mT
      //  - [x] 1 rateParam for QCD in high mT
      //  - [x] lnNs for the QCD OS/SS ratio
      //         * should account for stat + syst
      //         * systs should account for: extrap. from anti-iso to iso region,
      //           possible difference between ratio in low mT and high mT (overkill?)
      //  - [x] lnNs for the W+jets OS/SS ratio
      //         * should account for stat only if not being accounted for with bbb,
      //           i.e. because the OS/SS ratio was measured with a relaxed selection
      //         * systs should account for: changes in low/high mT and OS/SS due to JES
      //           and btag (if relevant); OS/SS being wrong in the MC (from enriched data?);
      //           low/high mT being wrong in the MC (fake rate dependence?)

      // Going to use the regex filtering to select the right subset of
      // categories for each rateParam
      cb.SetFlag("filters-use-regex", true);
      for (auto bin : cb_sig.cp().channel({"et", "mt"}).bin_set()) {
        // Regex that matches, e.g. mt_nobtag or mt_nobtag_X
        cb.cp().bin({bin+"(|_wjets_.*)$"}).process({"W"}).AddSyst(cb,
          "rate_W_cr_"+bin, "rateParam", SystMap<>::init(1.0));

        // Regex that matches, e.g. mt_nobtag or mt_nobtag_qcd_cr
        cb.cp().bin({bin+"(|_antiiso_)$"}).process({"QCD"}).AddSyst(cb,
          "rate_QCD_antiiso_"+bin, "rateParam", SystMap<>::init(1.0));

        // Regex that matches, e.g. mt_nobtag_wjets_cr or mt_nobtag_wjets_ss_cr
//        cb.cp().bin({bin+"_wjets_$"}).process({"QCD"}).AddSyst(cb,
//          "rate_QCD_highmT_"+bin, "rateParam", SystMap<>::init(1.0));
      }

        /////////////////
        // Systematics //
        /////////////////

      // Should set a sensible range for our rateParams
      for (auto sys : cb.cp().syst_type({"rateParam"}).syst_name_set()) {
        cb.GetParameter(sys)->set_range(0.0, 5.0);
      }
      cb.SetFlag("filters-use-regex", false);
    }
    
    
    
    
    
    if (zmm_fit) {
        cb.SetFlag("filters-use-regex", true);
        cb.cp().bin({"mt_nobtag","et_nobtag","em_nobtag","tt_nobtag"}).process({"ZTT"}).AddSyst(cb, "rate_ZMM_ZTT_nobtag", "rateParam", SystMap<>::init(1.0));
        cb.cp().bin({"zmm_nobtag"}).process({"ZL"}).AddSyst(cb, "rate_ZMM_ZTT_nobtag", "rateParam", SystMap<>::init(1.0));
        cb.cp().bin({"mt_btag","et_btag","em_btag","tt_btag"}).process({"ZTT"}).AddSyst(cb, "rate_ZMM_ZTT_btag", "rateParam", SystMap<>::init(1.0));
        cb.cp().bin({"zmm_btag"}).process({"ZL"}).AddSyst(cb, "rate_ZMM_ZTT_btag", "rateParam", SystMap<>::init(1.0));
        cb.GetParameter("rate_ZMM_ZTT_btag")->set_range(0.8, 1.2);
        cb.GetParameter("rate_ZMM_ZTT_nobtag")->set_range(0.95, 1.05);
        cb.SetFlag("filters-use-regex", false);
    }
  }
}
