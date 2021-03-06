#include "Sample.h"
#include "DiMuPlottingSystem.h"
#include "CutSet.h"
#include "Cut.h"
#include "SelectionCuts.h"
#include "CategorySelection.h"
#include "JetSelectionTools.h"

#include "EventTools.cxx"
#include "PUTools.cxx"
#include "SignificanceMetrics.cxx"

#include "TLorentzVector.h"

#include <sstream>
#include <map>
#include <vector>
#include <utility>

//////////////////////////////////////////////////////////////////
//---------------------------------------------------------------
//////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
    int input = 0;
    for(int i=1; i<argc; i++)
    {   
        std::stringstream ss; 
        ss << argv[i];
        if(i==1) ss >> input;
    }   

    // Not sure that we need a map if we have a vector
    // Should use this as the main database and choose from it to make the vector
    std::map<std::string, Sample*> samples;

    // Second container so that we can have a copy sorted by cross section.
    std::vector<Sample*> samplevec;

    // Use this to plot some things if we wish
    DiMuPlottingSystem* dps = new DiMuPlottingSystem();

  
    ///////////////////////////////////////////////////////////////////
    // SAMPLES---------------------------------------------------------
    ///////////////////////////////////////////////////////////////////

    float luminosity = 3990;

    float signalSF = 100;

    // ================================================================
    // Data -----------------------------------------------------------
    // ================================================================
 

    TString datafilename = 
    TString("/cms/data/store/user/t2/users/acarnes/h2mumu/samples/stage1/data/25ns/golden/CMSSW_8_0_X/stage_1_singleMuon_Run2016B_ALL.root");

    Sample* datasample = new Sample(datafilename, "Data", "data");
    datasample->lumi = luminosity;
    datasample->xsec = 9999;
    //datasample->pileupfile = "/cms/data/store/user/t2/users/acarnes/h2mumu/samples/stage1/data_from_json/25ns/golden/pileup/old/PUCalib_Golden_71mb.root";
    datasample->pileupfile = "pu_reweight_trees/8_0_X/PU_2016B_xsec71p3mb_CMSSW_8_0_X.root";
    samples["Data"] = datasample;

    // ================================================================
    // DYJetsToLL -----------------------------------------------------
    // ================================================================

    TString dyfilename   = TString("/cms/data/store/user/t2/users/acarnes/h2mumu/samples/stage1/mc/bg/dy/CMSSW_7_6_X/stage_1_dy_jetsToLL_ALL.root");
    samples["DYJetsToLL"] = new Sample(dyfilename, "DYJetsToLL", "background");
    samples["DYJetsToLL"]->pileupfile = "./pu_reweight_trees/7_6_X/PUCalib_DYJetsToLL.root"; //nPU
    samples["DYJetsToLL"]->xsec = 6025.2; // pb

/*
    // ================================================================
    // TTJets ---------------------------------------------------------
    // ================================================================

    TString ttbarfilename   = TString("/cms/data/store/user/t2/users/acarnes/h2mumu/samples/stage1/mc/bg/ttbar/CMSSW_7_6_X/stage_1_ttJets_ALL.root");
    samples["TTJets"] = new Sample(ttbarfilename, "TTJets", "background");
    samples["TTJets"]->pileupfile = "./pu_reweight_trees/7_6_X/PUCalib_TTJets.root"; //nPU
    samples["TTJets"]->xsec = 831.76; // pb

    // ================================================================
    // VBF ---------------------------------------------------------
    // ================================================================

    TString vbffilename   = TString("/cms/data/store/user/t2/users/acarnes/h2mumu/samples/stage1/mc/signal/CMSSW_7_6_X/stage_1_vbf_HToMuMu_ALL.root");
    samples["VBF"] = new Sample(vbffilename, "VBF", "signal");
    samples["VBF"]->pileupfile = "./pu_reweight_trees/7_6_X/PUCalib_VBF.root"; //nPU
    samples["VBF"]->xsec = 3.727*0.00022; // pb

    // ================================================================
    // GGF ---------------------------------------------------------
    // ================================================================

    TString ggfilename   = TString("/cms/data/store/user/t2/users/acarnes/h2mumu/samples/stage1/mc/signal/CMSSW_7_6_X/stage_1_gg_HToMuMu_ALL.root");
    samples["GGF"] = new Sample(ggfilename, "GGF", "signal");
    samples["GGF"]->pileupfile = "./pu_reweight_trees/7_6_X/PUCalib_GGF.root"; //nPU
    samples["GGF"]->xsec = 43.62*0.00022; // pb
*/

    ///////////////////////////////////////////////////////////////////
    // PREPROCESSING---------------------------------------------------
    ///////////////////////////////////////////////////////////////////

    // Loop through all of the samples to do some pre-processing
    std::cout << std::endl;
    std::cout << "======== Preprocess the samples... " << std::endl;
    std::cout << std::endl;

    //makePUHistos(samples);
    
    for(auto const &i : samples)
    {
        // Output some info about the current file
        std::cout << "  /// Preprocessing " << i.second->name << std::endl;
        std::cout << std::endl;
        std::cout << "    sample name:       " << i.second->name << std::endl;
        std::cout << "    sample file:       " << i.second->filename << std::endl;
        std::cout << "    pileup file:       " << i.second->pileupfile << std::endl;
        std::cout << "    nOriginal:         " << i.second->nOriginal << std::endl;
        std::cout << "    N:                 " << i.second->N << std::endl;
        std::cout << "    nOriginalWeighted: " << i.second->nOriginalWeighted << std::endl;
        std::cout << std::endl;

        if(!i.second->sampleType.Contains("data"))
        {
            // Pileup reweighting
            std::cout << "    +++ PU Reweighting " << i.second->name << "..."  << std::endl;
            std::cout << std::endl;

            i.second->lumiWeights = new reweight::LumiReWeighting(i.second->pileupfile.Data(), samples["Data"]->pileupfile.Data(), "pileup", "pileup");
            std::cout << "        " << i.first << "->lumiWeights: " << i.second->lumiWeights << std::endl;
            std::cout << std::endl;
        }
        samplevec.push_back(i.second);
    }

    // Sort the samples by xsec. Useful when making the histogram stack.
    std::sort(samplevec.begin(), samplevec.end(), [](Sample* a, Sample* b){ return a->xsec < b->xsec; }); 
    

    ///////////////////////////////////////////////////////////////////
    // Cut and Categorize ---------------------------------------------
    ///////////////////////////////////////////////////////////////////
    
    // Objects to help with the cuts and selections
    JetSelectionTools jetSelectionTools;
    CategorySelection categorySelection;
    Run1MuonSelectionCuts run1MuonSelection;
    Run1EventSelectionCuts run1EventSelection;

    TString varname;
    int bins;
    float min;
    float max;

    // recoCandMass
    if(input == 0)
    {
        bins = 150;
        min = 50;
        max = 200;
        varname = "dimuMass";
    }

    // dimuPt 
    if(input == 1)
    {
        bins = 200;
        min = 0;
        max = 100;
        varname = "dimuPt";
    }

    // recoPt
    if(input == 2)
    {
        bins = 200;
        min = 0;
        max = 150;
        varname = "recoMu_Pt";
    }
 
    // recoEta
    if(input == 3)
    {
        bins = 100;
        min = -2.5;
        max = 2.5;
        varname = "recoMu_Eta";
    }

    // NPV
    if(input == 4)
    {
        bins = 50;
        min = 0;
        max = 50;
        varname = "NPV";
    }

    std::cout << std::endl;
    std::cout << "======== Plot Configs ========" << std::endl;
    std::cout << "var         : " << varname << std::endl;
    std::cout << "min         : " << min << std::endl;
    std::cout << "max         : " << max << std::endl;
    std::cout << "bins        : " << bins << std::endl;
    std::cout << std::endl;

    TList* varlist = new TList();   // the list of histograms used to make the stack for var

    // Not sure how to deal with the scaling correctly when using a subset of events
    float reductionFactor = 1;

    for(auto const &s : samplevec)
    {
      // Output some info about the current file
      std::cout << std::endl;
      std::cout << "  /// Looping over " << s->name << std::endl;

      ///////////////////////////////////////////////////////////////////
      // HISTOGRAMS TO FILL ---------------------------------------------
      ///////////////////////////////////////////////////////////////////

      TH1F* varhisto = new TH1F(varname+"_"+s->name, varname+"_"+s->name, bins, min, max);

      for(unsigned int i=0; i<s->N/reductionFactor; i++)
      {

        ///////////////////////////////////////////////////////////////////
        // GET INFORMATION ------------------------------------------------
        ///////////////////////////////////////////////////////////////////

        s->getEntry(i); 
        s->vars.validJets = std::vector<TLorentzVector>();
        jetSelectionTools.getValidJetsdR(s->vars, s->vars.validJets);
        std::pair<int,int> e(s->vars.eventInfo.run, s->vars.eventInfo.event); // create a pair that identifies the event uniquely

        ///////////////////////////////////////////////////////////////////
        // CUTS  ----------------------------------------------------------
        ///////////////////////////////////////////////////////////////////

        if(!s->vars.reco1.isTightMuon || !s->vars.reco2.isTightMuon)
        { 
            continue; 
        }
        if(!run1EventSelection.evaluate(s->vars))
        { 
            continue; 
        }
        if(!run1MuonSelection.evaluate(s->vars)) 
        {
            continue; 
        }

        // recoCandMass
        if(varname.Contains("dimuMass")) 
        {
            float varvalue = -9999;
            varvalue = s->vars.recoCandMass;
            if(!(s->sampleType.Contains("data") && varvalue > 110 && varvalue < 140)) varhisto->Fill(varvalue, s->getWeight());   
        }

        // recoCandPt
        if(varname.Contains("dimuPt"))
             varhisto->Fill(s->vars.recoCandPt, s->getWeight());

        // recoMu_Pt
        if(varname.Contains("recoMu_Pt"))
        {
             varhisto->Fill(s->vars.reco1.pt, s->getWeight());
             varhisto->Fill(s->vars.reco2.pt, s->getWeight());
        }

        // recoMu_Eta
        if(varname.Contains("recoMu_Eta"))
        {
             varhisto->Fill(s->vars.reco1.eta, s->getWeight());
             varhisto->Fill(s->vars.reco2.eta, s->getWeight());
        }

        // NPV
        if(varname.Contains("NPV"))
        {
             varhisto->Fill(s->vars.vertices.nVertices, s->getWeight());
        }

        if(false)
          // ouput pt, mass info etc
          outputEvent(s->vars, categorySelection);

        // Reset the flags in preparation for the next event
        categorySelection.reset();
      }

      varhisto->Scale(s->getScaleFactor(luminosity));
      varlist->Add(varhisto);
    }

    // ////////////////////////////////////////////////////////////////////////////
    // ========= Scale, Stack, Save ===============================================
    // ////////////////////////////////////////////////////////////////////////////

    //TIter next(varlist);
    //TObject* object = 0;
    //while( (object = next()) )
    //{
    //  TH1F* varhisto = (TH1F*) object;
    //  if(TString(varhisto->GetName()).Contains("signal"))
    //  {
    //      // scale the signal so that it's easier to see on the plots
    //      // only do this right before saving or it would skew the significance results
    //      varhisto->Scale(signalSF);
    //  }
    //}

    // Create the stack and ratio plot    
    TCanvas* varstackcanvas = dps->stackedHistogramsAndRatio(varlist, "c_"+varname, varname+"_stack", varname, "Num Entries");
    std::cout << std::endl;

    std::cout << "  /// Saving plots..." << std::endl;
    std::cout << std::endl;
    TFile* savefile = new TFile("rootfiles/validate_"+varname+".root", "RECREATE");
    TDirectory* stacks = savefile->mkdir("stacks");
    TDirectory* histos = savefile->mkdir("histos");

    // save the different histos in the appropriate directories in the tfile
    stacks->cd();
    varstackcanvas->Write();

    histos->cd();
    varlist->Write();

    savefile->Close();

    return 0;
}
