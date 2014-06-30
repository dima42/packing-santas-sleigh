#include "Layer.h"

Layer::Layer(const MetaPresentV &presents, const int start_index, 
             const Genome &g, const int length, const int width,
             const int cutoff_count, int cap) : 
             m_length(length), m_width(width) {
        
    MetaPresentV max_presents(presents, start_index, cap);
    m_max_volume = max_presents.volume();
    
    int present_count = cap;
    //std::cout << cap << std::endl;
    while (present_count > cutoff_count) {
        MetaPresentV upcoming_presents(presents, start_index, present_count);
        
        MetaPresentV packed;
        //std::cout <<upcoming_presents.size() << std::endl; 
        
        if (present_count < 400) {
            m_meta_score = upcoming_presents.addMetaLevel();
	    int max_fit = maxPresentsByArea(upcoming_presents, 0, m_length*m_width);
            if (max_fit < upcoming_presents.size()){
                //no way they can fit
                present_count -= (upcoming_presents.size() - max_fit);
                continue;
            }
            upcoming_presents.sortByMaxXYSide(g.getOrderOffsets());
            g.rotatePresents(upcoming_presents);
            
            packed = packLB(upcoming_presents, g, false);
            //packed = packBF(upcoming_presents, g);
            //packed = packGuillotine(upcoming_presents, g);
            
            //MetaPresentV packed2 = packBF(upcoming_presents, g);
            //packed = packed.size()>packed2.size()? packed:packed2;
            //MetaPresentV packed3 = packGuillotine(upcoming_presents, g);        
            //packed = packed.size()>packed3.size()? packed:packed3;
            MetaPresentV packed4 = packLB(upcoming_presents, g, true);
            packed = packed.size()>packed4.size()?packed:packed4;
            packed.unravel();
        }else{
            g.rotatePresents(upcoming_presents);
            upcoming_presents.sortByMaxXYSide(g.getOrderOffsets());
            packed = packGuillotine(upcoming_presents, g);
        }
        
        //std::cout << packed.size() << " " << present_count << std::endl;
        if ((packed.size() == present_count) ){ 
            //all presents packed
            m_unraveled_presents = packed;
            
            
            if (present_count == cap and 
                (present_count < presents.size()-start_index) ){
                //the cap is not hard because of metalevels
                cap += 1;
                present_count += 1;
            }else{
                break;
            }
        }else{
            present_count -= 1;
        }
    }    
    
    m_unraveled_presents.sortByOrder();
    
}

int Layer::maxPresentsByArea(const MetaPresentV &presents, 
                             const int start_index, const int area){
    
    int count = -1;
    int remaining_area = area;
    while ( remaining_area >= 0 ){
        count += 1;
        //increment should happen on the last present because count starts at -1
        
        if ((start_index + count) >= presents.size()){
            break;
        }
        MetaPresent p = presents.getMetaPresent(start_index+count);
        double a1 = p.getSide(0)*p.getSide(1);
        double a2 = p.getSide(0)*p.getSide(2);
        double a3 = p.getSide(1)*p.getSide(2);
        a1 = a1<a2?a1:a2;
        a1 = a1<a3?a1:a3;
        remaining_area -= a1;
    }
    //std::cout << count << " " << start_index << std::endl;
    return count;
}

double Layer::getMetric() const{
    return m_unraveled_presents.volume()/m_max_volume;     
}

MetaPresentV Layer::packGuillotine(const MetaPresentV &presents, const Genome &g){
    //TODO: we could actually consider fit instead of sorting presents in order
    MetaPresentV present_copy = presents;
    MetaPresentV placed_presents;
    
    std::vector<Coords> free_rects; //only x/y will be filled in
    //TODO: switch to something better than vector since g is doing 
    //custom insertion
    Coords base_rect;
    base_rect.min.push_back(1);
    base_rect.min.push_back(1);
    base_rect.min.push_back(-1);
    base_rect.max.push_back(m_length);
    base_rect.max.push_back(m_width);
    base_rect.max.push_back(-1);
    free_rects.push_back(base_rect);
    
    int i = 0;
    while(i < presents.size()){
        //get next present
        MetaPresent p = present_copy.getMetaPresent(i++);
        
        std::vector<Coords>::iterator it = free_rects.begin();
        while (it != free_rects.end()){
            int rect_x = it->max[0] - it->min[0] + 1;
            int rect_y = it->max[1] - it->min[1] + 1;
            
            //see if present needs to be rotated again
            //this is basically just assisting the rotation genetics
            //TODO: should we think about entangling rotation with rect_x/rect_y
            //even when the present fits as-is?
            /*UPDATE
            Guillotine is no longer allowed to rotate because of pointer issues
            when running multiple heuristic methods on non-leaf metapresents
            I guess in theory this can be fixed but way too much work.
            */
            if ( ((p.getSide(0) > rect_x) or (p.getSide(1) > rect_y)) and 
                 ((p.getSide(1) <= rect_x) and (p.getSide(0) <= rect_y)) ){
                //Rotation prev_r = p.getRotation();
                //p.rotate({prev_r[1], prev_r[0], prev_r[2]});
                
            }
            
            //see if it fits
            if ((p.getSide(0) <= rect_x) and (p.getSide(1) <= rect_y)){
                
                Coords fc = *it;
                Coords bl;
                bl.min.push_back(it->min[0]);
                bl.min.push_back(it->min[1]);
                bl.min.push_back(-1);
                bl.max.push_back(it->min[0] + p.getSide(0) - 1);
                bl.max.push_back(it->min[1] + p.getSide(1) - 1);
                bl.max.push_back(-1);
                
                std::pair<Coords, Coords> new_rects = g.rectSplit(fc, bl);
                //need to get new rects before flipping
                
                //TODO If we add rectangle merging, there could be 
                //incentive to prioritize consistent corner practices.                
                Coords bestc = bl;
                bool xf = false;
                bool yf = false;
                //TODO: add corner selection
                
                //bestc.flip(xf, yf, fc);
                //new_rects.first.flip(xf, yf, fc);
                //new_rects.second.flip(xf, yf, fc);
                
                free_rects.erase(it); //order important here - erase first
                if ((new_rects.first.max[0]-new_rects.first.min[0] >= 0) and
                    (new_rects.first.max[1]-new_rects.first.min[1] >= 0) ){
                    g.insertRect(free_rects, new_rects.first);
                }
                if ((new_rects.second.max[0]-new_rects.second.min[0] >= 0) and
                    (new_rects.second.max[1]-new_rects.second.min[1] >= 0) ){
                    g.insertRect(free_rects, new_rects.second);
                }
                
                
                //place present
                p.place(bestc);
                placed_presents.insert(p);
                
                //TODO rect merge would go here
                break;
            }
            ++it;
        }
        Coords c = p.getCoords();
        if (c.min[0] < 0){ 
            //unable to place present
            break;
        }
        
    }
    
    return placed_presents;
}


MetaPresentV Layer::packBF(const MetaPresentV &presents, const Genome &g){
                    
    MetaPresentV placed_presents;
    MetaPresentV present_copy = presents;
    
    std::vector<int> skyline(m_length, 0);
    //warning: skyline x values are offset by one due to array indices
    //shifting only occurs when present coords are set
    
    while (present_copy.size()>0){
    
        //find lowest gap
        //if two gaps are at same height it's ok to find either one only
        //since we will fill in that gap if no present fits
        int lowest_gap_minx = 0;
        int lowest_gap_height = 100000;
        int lowest_gap_length = 0;
        int examine_index = 0;
        bool streak_going = false;
        while (examine_index < skyline.size()){
            if (skyline[examine_index] < lowest_gap_height){
                //start new streak
                lowest_gap_minx = examine_index;
                lowest_gap_length = 1;
                lowest_gap_height = skyline[examine_index];
                streak_going = true;
            }else if ((skyline[examine_index] == lowest_gap_height) and 
                       streak_going){
                //continue old streak
                lowest_gap_length += 1;
            } else{
                streak_going = false;
            }
            examine_index++;
        }
        
        //find lowest neighbor height
        int lowest_neighbor_height = m_width; 
        int left_neighbor_index = lowest_gap_minx-1;
        if (lowest_gap_minx > 0){
            if (skyline[left_neighbor_index] < lowest_neighbor_height ){
                lowest_neighbor_height = skyline[left_neighbor_index];
            }
        }
        int right_neighbor_index = lowest_gap_minx + lowest_gap_length;
        if (right_neighbor_index < skyline.size()){
            if (skyline[right_neighbor_index] < lowest_neighbor_height){
                lowest_neighbor_height = skyline[right_neighbor_index];
            }
        }
        
        //find best fitting present
        int best_fit_present_i = -1; //not found yet
        double best_fit_score = 0;
        int search_cap = present_copy.size();
        MetaPresent first_p = present_copy.getMetaPresent(0);
        
        for (int i = 0; i < search_cap; i++){
            const MetaPresent& p = present_copy.getMetaPresent(i);
            if ((lowest_gap_height + p.getSide(1))>= m_width){
                //this present will never fit; avoid further computation
                break;
            }
            
            if ((p.getSide(0) > lowest_gap_length) or 
                ((lowest_gap_height + p.getSide(1))>= m_width)){
                //doesn't fit in gap
                continue;
            }
                        
            double gap_height = (lowest_neighbor_height-lowest_gap_height);
            if ((p.getSide(1) > gap_height) and 
                ((p.getSide(1)) < 0.9*(first_p.getSide(1)) ) ){
                //not efficient use of space
                continue;
            }            
            
            double fit_score = p.getSide(0)*100.0+p.getSide(1);;
            if (fit_score > best_fit_score){
                best_fit_score = fit_score;
                best_fit_present_i = i;
            }
        }
        
        //no present fits?  update skyline and break if necessary
        if (best_fit_present_i == -1){
            if (lowest_neighbor_height == m_width){
                //skyline can't be widened so no more presents can be placed
                break;
            } else{
                for (int i = lowest_gap_minx; 
                    i < lowest_gap_minx + lowest_gap_length; i++){
                    //strict inequality because gap length starts at 1, not 0
                    skyline[i] = lowest_neighbor_height;
                }
            }
        }
        
        //present fits?  place present
        else{            
            MetaPresent p = present_copy.getMetaPresent(best_fit_present_i);
            present_copy.erase(best_fit_present_i);
                        
            int minx = lowest_gap_minx;
            int maxx = lowest_gap_minx + p.getSide(0) - 1;
            
            //update skyline
            for (int i = minx; i <= maxx; i++){
                skyline[i] += p.getSide(1);
            }
                        
            //add present to placed_presents                
            Coords c;
            c.min.push_back(minx+1);
            c.min.push_back(lowest_gap_height+1);
            c.min.push_back(-1);
            
            c.max.push_back(maxx+1);
            c.max.push_back(lowest_gap_height+p.getSide(1));
            c.max.push_back(-1);
            
            p.place(c);            
            placed_presents.insert(p);
            
        }                   
    }
    return placed_presents;
}

MetaPresentV Layer::packLB(const MetaPresentV &presents, const Genome &g, bool try_both){
    
    MetaPresentV placed_presents;
    MetaPresentV present_copy = presents;

    std::set<int> constx_boundaries;
    std::set<int> consty_boundaries;
    
    constx_boundaries.insert(0);
    constx_boundaries.insert(1001);
    consty_boundaries.insert(0);
    consty_boundaries.insert(1001);
    
    
    int current_i = 0;
    int max_y = 0;
    int area_sum = 0;
    std::set<int> heights;
    int look_ahead = 10;
    

    while (current_i < presents.size()){
        
        int best_index = 0;
        double best_efficiency = -1000000;
        Coords best_c;
        bool placement_possible = false;
        int lowest_height = 0;

        MetaPresent next = present_copy.getMetaPresent(0);
        int first_side = next.getSide(0)>next.getSide(1)?next.getSide(0):next.getSide(1);
        if (first_side > 70){
            look_ahead = 3;
        }else{
            look_ahead = 1;
        }
        
        //find best present to place
        for (int i = 0; (i < present_copy.size()) and  (i < look_ahead); i++){
            
            MetaPresent p = present_copy.getMetaPresent(i);
            Coords c1, c2, c3, c4;
            
            //find the bottomest bl corner where present would fit
            c1 = lh::findLB(p, placed_presents, constx_boundaries, consty_boundaries, std::make_pair(m_length, m_width));
            
            double eff1 = 0.02*p.getSide(0)+0.02*p.getSide(1)+10000.0/(c1.min[1]+c1.min[0]-1);
            
            Coords c = c1;
            double efficiency = eff1;
            
            if (try_both){
                c2 = lh::findRT(p, placed_presents, constx_boundaries, consty_boundaries, std::make_pair(m_length, m_width));                
                double eff2 = 0.02*p.getSide(0)+0.02*p.getSide(1)+10000.0/(2001-c2.max[1]-c2.max[0]);                
                if ( (eff2>eff1 and (c2.min[0] != -1)) or c1.min[0] == -1){
                    c = c2;
                    efficiency = eff2;
                }
            }
            if (c.min[0] == -1){
                //present can't be placed
                //std::cout << "Can't place" << c.min[0] << "," << c.min[1] << " " << c.max[0] << "," << c.max[1] << " " << p.getOrder() << std::endl;
                return placed_presents;
            }
            if (efficiency>best_efficiency){
                //present found and better than others
                //std::cout << best_index << " " << p.getSide(0) << " " << efficiency << " " << best_efficiency << std::endl;
                best_index = i;
                best_efficiency = efficiency;
                best_c = c;
            }
        }
        
       
        
        //std::cout << current_i << " ";//std::endl;
        MetaPresent p = present_copy.getMetaPresent(best_index);
        //std::cout << best_c.min[0] << "," << best_c.min[1] << " " << best_c.max[0] << "," << best_c.max[1] << " " << p.getOrder() << std::endl;
        Coords c = best_c;
        present_copy.erase(best_index);
        area_sum += p.getSide(0)*p.getSide(1);
        if (c.max[1] > max_y) max_y = c.max[1];
        
        
        //add points to grid
        if (try_both){
            constx_boundaries.insert(c.min[0]);
            consty_boundaries.insert(c.min[1]);
        }
        constx_boundaries.insert(c.max[0]);
        consty_boundaries.insert(c.max[1]);
        
        //insert present into placing
        p.place(c);            
        placed_presents.insert(p);
        heights.insert(c.min[1]);
        current_i += 1;
    }
    
    return placed_presents;
}

