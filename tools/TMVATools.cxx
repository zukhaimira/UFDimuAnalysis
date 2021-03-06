///////////////////////////////////////////////////////////////////////////
//                           TMVATools.cxx                              //
//=======================================================================//
//                                                                       //
//        Miscellaneous tools to help with TMVA Classification.          //
//                                                                       //
///////////////////////////////////////////////////////////////////////////

#include "TMVATools.h"

//////////////////////////////////////////////////////////////////
//---------------------------------------------------------------
//////////////////////////////////////////////////////////////////

void TMVATools::getNamesRecursive(TXMLEngine* xml, XMLNodePointer_t node, TString node_name, TString node_att, std::vector<TString>& names)
{
// Get the variable names that were used in the training for this classification model
// tvars are the variables used in the training, svars are the spectator variables

   TString nname = xml->GetNodeName(node);
   //std::cout << "node: " << node_name << std::endl;
   
   // display attributes
   XMLAttrPointer_t attr = xml->GetFirstAttr(node);
   while (attr!=0) 
   {
       TString att_string = xml->GetAttrName(attr);
       TString val_string = xml->GetAttrValue(attr);
       if(nname == node_name && att_string == node_att)
       {
           names.push_back(val_string);
           //printf("attr: \"%s\" value: \"%s\"\n", att_string.Data(), val_string.Data());
       }
       attr = xml->GetNextAttr(attr);
   }
   
   // display all child nodes
   XMLNodePointer_t child = xml->GetChild(node);
   while (child!=0) 
   {
       getNamesRecursive(xml, child, node_name, node_att, names);
       child = xml->GetNext(child);
   }   
}

//////////////////////////////////////////////////////////////////
//---------------------------------------------------------------
//////////////////////////////////////////////////////////////////

void TMVATools::getNames(TString filename, TString node_name, TString node_att, std::vector<TString>& names)
{
// Get the variable names that were used in the training for this classification model
// tvars are the variables used in the training, svars are the spectator variables

    // First create the engine.
    TXMLEngine* xml = new TXMLEngine;

    // Now try to parse xml file.
    XMLDocPointer_t xmldoc = xml->ParseFile(filename);
    if (xmldoc==0)
    {
        delete xml;
        return;
    }

    // Get access to main node of the xml file.
    XMLNodePointer_t mainnode = xml->DocGetRootElement(xmldoc);

    // Recursively connect nodes together.
    getNamesRecursive(xml, mainnode, node_name, node_att, names);

    // Release memory before exit
    xml->FreeDoc(xmldoc);
    delete xml;
}

//////////////////////////////////////////////////////////////////
//---------------------------------------------------------------
//////////////////////////////////////////////////////////////////

void TMVATools::getVarNames(TString filename, std::vector<TString>& tvars, std::vector<TString>& svars)
{
// Get the variable names that were used in the training for this classification model
// tvars are the variables used in the training, svars are the spectator variables

    // Recursively connect nodes together.
    getNames(filename, "Variable", "Label", tvars);
    getNames(filename, "Spectator", "Label", svars);
}

//////////////////////////////////////////////////////////////////
//---------------------------------------------------------------
//////////////////////////////////////////////////////////////////

void TMVATools::getClassNames(TString filename, std::vector<TString>& classes)
{
// Get the variable names that were used in the training for this classification model
// tvars are the variables used in the training, svars are the spectator variables

    // Recursively connect nodes together.
    getNames(filename, "Class", "Name", classes);
}


//////////////////////////////////////////////////////////////////
//---------------------------------------------------------------
//////////////////////////////////////////////////////////////////

TMVA::Reader* TMVATools::bookVars(TString methodName, TString weightfile, std::map<TString, Float_t>& tmap, std::map<TString, Float_t>& smap)
{
// Map the variable name to its float value, give the reader the variable names and the addresses for the input variables.

   std::vector<TString> tvars;
   std::vector<TString> svars;
   getVarNames(weightfile, tvars, svars);

   // --- Create the Reader object
   // Vars need to have the same names as in the xml
   // and they need to be booked in the same order

   TMVA::Reader *reader = new TMVA::Reader("!Color:!Silent");    

   for(auto& var: tvars)
   {   
       tmap[var] = -999;
       reader->AddVariable(var, &tmap[var]);
   }   

   //for(auto& item: tmap)
   //{   
   //    printf("%s: %f\n", item.first.Data(), item.second);
   //}   

   // dumb... Spectator variables declared in the training have to be added to the reader, too
   for(auto& var: svars)
   {   
       smap[var] = -999;
       reader->AddSpectator(var, &smap[var]);
   }   

   // Book BDT method into reader, now that the vars are set up
   reader->BookMVA( methodName, weightfile );  

   return reader;
}

//////////////////////////////////////////////////////////////////
//---------------------------------------------------------------
//////////////////////////////////////////////////////////////////

float TMVATools::getClassifierScore(TMVA::Reader* reader, TString methodName, std::map<TString, Float_t>& tmap, VarSet& varset)
{
// assuming that the reader has been initialized with the tmva weight file
// and the appropriate vars, get the classifier score

      //std::cout << Form("\n  /// Set map values %s \n\n", s->name.Data());
      for(auto& v: tmap)
      {   
          tmap[v.first] = varset.getValue(v.first.Data());
          //printf("  %s: %f\n", v.first.Data(), v.second);
      }   

      //std::cout << std::endl << Form("  /// Get BDT Output %s \n", s->name.Data());
      return reader->EvaluateMVA(methodName);
}

//////////////////////////////////////////////////////////////////
//---------------------------------------------------------------
//////////////////////////////////////////////////////////////////

std::vector<float> TMVATools::getMulticlassScores(TMVA::Reader* reader, TString methodName, std::map<TString, Float_t>& tmap, VarSet& varset)
{
// assuming that the reader has been initialized with the tmva weight file
// and the appropriate vars, get the classifier score

      //std::cout << Form("\n  /// Set map values %s \n\n", s->name.Data());
      for(auto& v: tmap)
      {   
          tmap[v.first] = varset.getValue(v.first.Data());
          //printf("  %s: %f\n", v.first.Data(), v.second);
      }   

      //std::cout << std::endl << Form("  /// Get BDT Output %s \n", s->name.Data());
      return reader->EvaluateMulticlass(methodName);
}

