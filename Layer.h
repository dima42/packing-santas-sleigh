#pragma once

#include <cmath>
#include <map>
#include <set>
#include <utility>
#include <vector>

#include "Genetics.h"
#include "LayerHelper.h"
#include "MetaPresent.h"

#include <iostream>

class Layer{
public:
    Layer(const MetaPresentV &presents, const int start_index, const Genome &g, 
          const int length, const int width, const int cutoff_count, int cap);
    //packs a layer of presents.
    //the plan is to repeatedly call packBF/packGuillotine on ordered subsets
    //of presents and thus determine the maximal number of presents that
    //fit in a layer.
    
    Layer(){};
    //creates a meaningless layer to be overwritten later
    
    MetaPresentV getMetaPresents() {return m_unraveled_presents;}
    //gets the presents packed into this layer.
    //the presents will have x/y set, but no z.
    
    static int maxPresentsByArea(const MetaPresentV &presents, 
                                 const int start_index, const int area);
    //determines an upper bound count on presents for this layer by setting
    //present minimal face area to sum to m_length*m_width
    
    double getMetric() const;
    
    double m_meta_score;
    
private:

    MetaPresentV packGuillotine(const MetaPresentV &presents, const Genome &g);
    //Packs top level of metapresents
    //returns a MetaPresentV with x/y filled out.  the presents are packed
    //by the Guillotine procedure with parameters as determined by g.
    //the presents are placed in the order received.
    //m_heights and m_presents are not updated in this procedure.
    
    MetaPresentV packBF(const MetaPresentV &presents, const Genome &g);
    //Packs top level of metapresents
    //uses Burke, Kendall and Whitwell's 2004 algorithm
    //we use a slightly less greedy approach: small presents 
    //are only placed if they do not increase fragmentation          
    
    MetaPresentV packLB(const MetaPresentV &presents, const Genome &g, bool try_both);
    //Packs top level of metapresents
    //Puts present in position closest to (0, 0)
        

    
    int m_length, m_width;
    
    MetaPresentV m_unraveled_presents;
    //result is unraveled and filled out after layering filling finished
    
    double m_max_volume;
    
    
};