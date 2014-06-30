#include "LayerHelper.h"



namespace lh{

Coords findLB(const MetaPresent &p, const MetaPresentV &placed_presents, const std::set<int> &constx_boundaries, const std::set<int> &consty_boundaries, std::pair<int, int> layer_size){
    Coords c;
    
    std::set<int>::iterator xit, yit;
    bool found = false;
    int best_found_score = 100000000;
    Coords candidate;
    candidate.min.push_back(0);
    candidate.min.push_back(0);
    candidate.min.push_back(-1);
    candidate.max.push_back(0);
    candidate.max.push_back(0);
    candidate.max.push_back(-1);
    for (std::set<int>::const_iterator lxit=constx_boundaries.begin(); lxit != --constx_boundaries.end(); ++lxit){
        int x_val = *lxit;
        
        
        if (x_val > best_found_score){
            break;
        }
        for (std::set<int>::const_iterator byit = consty_boundaries.begin(); byit != --consty_boundaries.end(); ++byit){
            
            int y_val = *byit;
            if ( ((y_val-1) + (x_val-1)) > best_found_score){
                break;
            }
            if ( ((x_val + p.getSide(0)) > layer_size.first)  or 
                 ((y_val+p.getSide(1) )>layer_size.second)       ){
                 break;
            }
                        
            candidate.min[0] = x_val+1;
            candidate.min[1] = y_val+1;
            candidate.max[0] = x_val+p.getSide(0);
            candidate.max[1] = y_val+p.getSide(1);
            
            
            if (!placed_presents.collides(candidate)){
                //already know we have best score from earlier check
                found = true;
                best_found_score = (x_val-1)+(y_val-1);
                c = candidate;
                break; //don't need to check other y's
            }
        }
    }
    
    if (!found){
        for (int i = 0; i < 3; i++){
            c.max.push_back(-1);
            c.min.push_back(-1);
        } //signal that no space found
    }
    
    return c;
}//end findLB

Coords findRT(const MetaPresent &p, const MetaPresentV &placed_presents, std::set<int> &constx_boundaries, std::set<int> &consty_boundaries, std::pair<int, int> layer_size){
    Coords c;
    
    std::set<int>::iterator xit, yit;
    bool found = false;
    int best_found_score = 100000;
    Coords candidate;
    candidate.min.push_back(0);
    candidate.min.push_back(0);
    candidate.min.push_back(-1);
    candidate.max.push_back(0);
    candidate.max.push_back(0);
    candidate.max.push_back(-1);
    for (std::set<int>::const_iterator lxit=--constx_boundaries.end(); lxit != constx_boundaries.begin(); --lxit){
        int x_val = *lxit;
        
        
        if (found and ((1000-x_val) > best_found_score)){
            break;
        }
        for (std::set<int>::const_iterator byit = --consty_boundaries.end(); byit != consty_boundaries.begin(); --byit){
            
            int y_val = *byit;
            if (found and (1000-y_val)+(1000-x_val) > best_found_score){
                break;
            }
            if ( ((x_val - p.getSide(0)+1) < 0)  or 
                 ((y_val-p.getSide(1)+1 ) < 0)       ){
                 break;
            }
            
            
	    candidate.max[0] = x_val-1;
	    candidate.max[1] = y_val-1;
	    candidate.min[0] = x_val-p.getSide(0);
	    candidate.min[1] = y_val-p.getSide(1);
            
            if (!placed_presents.collides(candidate)){
                found = true;
                best_found_score = (1000-x_val)+(1000-y_val);
                c = candidate;
                break;
            }
        }
    }
    
    if (!found){
        for (int i = 0; i < 3; i++){
            c.max.push_back(-1);
            c.min.push_back(-1);
        } //signal that no space found
    }
    
    return c;

}//end findRT

}//end namespace lh
