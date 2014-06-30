#include "MetaPresent.h"

void Coords::flip(const bool flip_x, const bool flip_y, 
                  const Coords &flip_coords){
    std::vector<int> tmpmin = min;
    std::vector<int> tmpmax = max;
    if (flip_x){
        min[0] = flip_coords.max[0]-tmpmax[0]+flip_coords.min[0];
        max[0] = flip_coords.max[0]-tmpmin[0]+flip_coords.min[0];
    }
    if (flip_y){
        min[1] = flip_coords.max[1]-tmpmax[1]+flip_coords.min[1];
        max[1] = flip_coords.max[1]-tmpmin[1]+flip_coords.min[1];
    }
}

MetaPresent::MetaPresent(const int o, const int a, const int b, const int c, 
                         const Rotation &r){
    m_order = o;
    m_order_offset = 0;
    m_lwh = {a, b, c};
    m_sorted_lwh = m_lwh;
    std::sort(m_sorted_lwh.begin(), m_sorted_lwh.end());
    for (int i = 0; i < 3; i++){
        m_coords.min.push_back(-1);
        m_coords.max.push_back(-1);    
    }
    m_rotation = r;
    m_leaf = true;
    rotate(r);
}

MetaPresent::MetaPresent(MetaPresent lower, MetaPresent upper, int merge_height){
    m_order = -1; //only first order meta for now so order doesn't matter
    m_order_offset = 0;
    int al = lower.getSide(0);
    int bl = lower.getSide(1);
    int au = upper.getSide(0);
    int bu = upper.getSide(1);
    //std::cout << upper.getOrder() << "," << lower.getOrder() << ": "  << " " << au << " " << bu << al << " " << bl << std::endl;
    m_lwh = {al>au?al:au, bl>bu?bl:bu, merge_height};
    //std::cout << m_lwh[0] << " " << m_lwh[1] << std::endl << std::endl;
    m_rotation = {m_lwh[0]>=m_lwh[1], m_lwh[0]<m_lwh[1], 2};
        
    for (int i = 0; i < 3; i++){
        m_coords.min.push_back(-1);
        m_coords.max.push_back(-1);    
    }
    //the metapresent is not placed yet
    
    m_leaf = false;

    m_lower_x_cursor = 1;  //lowest x available to place
    m_lower_y_cursor = 1;
    m_upper_x_cursor = 1;
    m_upper_y_cursor = 1;
    
    m_split_height = merge_height - upper.getSide(2);
    
    addChild(lower, true);
    addChild(upper, false);
    
}

bool MetaPresent::fitChild(const MetaPresent &child, bool lower){
    //simple shelving, todo improve if seems like it might be worth it
    
    int avail_height = lower?m_split_height:(m_lwh[2]-m_split_height);
    if (child.getSide(2) > avail_height){
        return false;
    }
    if (child.getSide(0) > m_lwh[0]){
        return false;
    }
        
    
    int x_cursor = lower?m_lower_x_cursor:m_upper_x_cursor;
    int y_cursor = lower?m_lower_y_cursor:m_upper_y_cursor;        
    
    if ((x_cursor + child.getSide(0) -1) > m_lwh[0]){
        x_cursor = 1;
        y_cursor = lower?m_lower_max_y: m_upper_max_y;
    }
    
    if ((y_cursor + child.getSide(1)-1) > m_lwh[1]){
        return false;
    }
    return true;
}

bool MetaPresent::addChild(MetaPresent child, bool lower){
    //simple shelving, todo improve if seems like it might be worth it
    
    int avail_height = lower?m_split_height:(m_lwh[2]-m_split_height);
    if (child.getSide(2) > avail_height){
        return false;
    }
    if (child.getSide(0) > m_lwh[0]){
	return false;
    }
        
    
    int x_cursor = lower?m_lower_x_cursor:m_upper_x_cursor;
    int y_cursor = lower?m_lower_y_cursor:m_upper_y_cursor;        
    
    if ((x_cursor + child.getSide(0) -1) > m_lwh[0]){
        x_cursor = 1;
        y_cursor = lower?m_lower_max_y: m_upper_max_y;
    }
    
    if ((y_cursor + child.getSide(1)-1) > m_lwh[1]){
        return false;
    }
    
    Coords c;
    c.min.push_back(x_cursor);
    c.min.push_back(y_cursor);
    c.min.push_back(-1);
    
    c.max.push_back(x_cursor + child.getSide(0)-1);
    c.max.push_back(y_cursor + child.getSide(1)-1);
    c.max.push_back(-1);
    x_cursor = c.max[0] + 1;
    child.place(c);
    m_children_ptrs.push_back(std::make_shared<MetaPresent>(child));    
    
    if (lower){
    
        m_lower_coords.push_back(c);
        m_lower_x_cursor = x_cursor;
        m_lower_max_y = (y_cursor + child.getSide(1)) > m_lower_max_y? 
                        (y_cursor + child.getSide(1)) : m_lower_max_y;
        m_lower_y_cursor = y_cursor;
    }else{
        m_upper_coords.push_back(c);
        m_upper_x_cursor = x_cursor;
        m_upper_max_y = (y_cursor + child.getSide(1)) > m_upper_max_y? 
                        (y_cursor + child.getSide(1)) : m_upper_max_y;
        m_upper_y_cursor = y_cursor;
    }
    /*
    std::cout << c.min[0] << " " << c.min[1] << std::endl;
    std::cout << c.max[0] << " " << c.max[1] << std::endl << std::endl; 
    std::cout << m_lwh[0] << " " <<m_lwh[1] << std::endl;
    */
    return true;       
}

void MetaPresent::rotate(const Rotation &r, bool set){


    if (!m_leaf){
	
        if (r[2] != 2){
            std::cout << "you're trying to invalidly rotate a metapresent";
            return;
        }
        if (m_rotation == r){
            return;
        }
        
        //a rotation here will just be a flip of x's and y's for all the children
        for(auto it = m_children_ptrs.begin(); 
                 it != m_children_ptrs.end(); ++it){
            Coords c = (*it)->getCoords();
            
         
            c.min[0] = c.min[1];
            c.min[1] = m_lwh[0]-c.max[0]+1;
            c.max[0] = c.min[0] + (*it)->getSide(1)-1;
            c.max[1] = c.min[1]+(*it)->getSide(0)-1;
            Rotation cr = (*it)->getRotation();
            Rotation crn = cr;
            crn[0] = cr[1];
            crn[1] = cr[0];
            (*it)->rotate(crn);           
            //guaranteed to be different because rotate() escapes when it's not
            (*it)->place(c);
        }         
        Rotation mrt = m_rotation;
        m_rotation[1] = mrt[0];
        m_rotation[0] = mrt[1];
        int tmp = m_lwh[0];
        m_lwh[0] = m_lwh[1];
        m_lwh[1] = tmp;
        //can't use sorted here because bad things happen when z < x/y
        
        return;
    }
    //todo: is the above ok?
    
    for(int i = 0; i < 3; i++){
        m_lwh[i] = m_sorted_lwh[r[i]];
    }
    
    //if (set){ 
        m_rotation = r;
    //}//some sort of weird bug
    
}

std::vector<std::shared_ptr<MetaPresent>> MetaPresent::getBasePointers(){
    std::vector<std::shared_ptr<MetaPresent>> children; 
    if (m_leaf){
        children.push_back(std::shared_ptr<MetaPresent>(this));
    }else{        
        for(auto it = m_children_ptrs.begin(); 
                 it != m_children_ptrs.end(); ++it){
            std::vector<std::shared_ptr<MetaPresent>> child_ptrs;
            //child_ptrs = (*it)->getBasePointers();
            //there is some weird memory stuff going on if we try
            //to do this recursively.  but only 1 level so far so no 
            //need to debug yet
            /*
            children.insert(children.end(), 
                            child_ptrs.begin(), child_ptrs.end());
            */  
            children.insert(children.end(), *it);
                      
        }
    }
    return children;
}


MetaPresentV::MetaPresentV(){
}

MetaPresentV::MetaPresentV(const std::string &filename, const int count){
    std::ifstream presentfile(filename);
    std::string line;
    std::getline(presentfile, line); //header
    int skipped = 0;
    for(int i = 0; i < skipped; i++) std::getline(presentfile, line);
    for (int i = 0; i < count; i++){
        std::getline(presentfile, line);
        std::stringstream ss(line);
        char comma;
        int o, a, b, c;
        ss>>o>>comma>>a>>comma>>b>>comma>>c;
        
        Rotation r;
        r.push_back(0);
        r.push_back(1);
        r.push_back(2);
        int bound = 0;
        if (a >= bound and b >= bound and c >= bound){
            m_metapresents.push_back( MetaPresent(o - skipped, a, b, c, r));
        }else{
            skipped +=1;
        }
    }
    Rotation r1, r2, r3, r4, r5, r6;

    r1.push_back(1);r1.push_back(2);r1.push_back(0);
    r2.push_back(2);r2.push_back(1);r2.push_back(0);
    r3.push_back(2);r3.push_back(0);r3.push_back(1);
    r4.push_back(0);r4.push_back(2);r4.push_back(1);
    r5.push_back(0);r5.push_back(1);r5.push_back(2);
    r6.push_back(1);r6.push_back(0);r6.push_back(2);

    m_possible_rots.push_back(r1);
    m_possible_rots.push_back(r2);
    m_possible_rots.push_back(r3);
    m_possible_rots.push_back(r4);
    m_possible_rots.push_back(r5);
    m_possible_rots.push_back(r6);

   
}

MetaPresentV::MetaPresentV(const MetaPresentV &parent, const int start_index, 
                   const int count){
    for (int i = 0; i < count; i++){
        m_metapresents.push_back(parent.getMetaPresent(i+start_index));
    }
    Rotation r1, r2, r3, r4, r5, r6;

    r1.push_back(1);r1.push_back(2);r1.push_back(0);
    r2.push_back(2);r2.push_back(1);r2.push_back(0);
    r3.push_back(2);r3.push_back(0);r3.push_back(1);
    r4.push_back(0);r4.push_back(2);r4.push_back(1);
    r5.push_back(0);r5.push_back(1);r5.push_back(2);
    r6.push_back(1);r6.push_back(0);r6.push_back(2);

    m_possible_rots.push_back(r1);
    m_possible_rots.push_back(r2);
    m_possible_rots.push_back(r3);
    m_possible_rots.push_back(r4);
    m_possible_rots.push_back(r5);
    m_possible_rots.push_back(r6);

}

double MetaPresentV::volume() const{
    double running_volume=0;
    for (auto it = m_metapresents.begin(); it != m_metapresents.end(); ++it){
        running_volume += it->getSide(0)*it->getSide(1)*it->getSide(2);
    }
    return running_volume;
}

bool MetaPresentV::collides(const Coords &c) const{
    //this is called all the time, don't make it slow
    
    for (auto it = m_metapresents.begin(); it != m_metapresents.end(); ++it){
        if ((it->getCoords().min[0] <= c.max[0] and it->getCoords().max[0] >= c.min[0]) and 
            (it->getCoords().min[1] <= c.max[1] and it->getCoords().max[1] >= c.min[1]) ){
            return true;
        }
    }
    return false;
}

void MetaPresentV::insert(const MetaPresentV &new_mpv){  
    std::vector<MetaPresent> new_mps = new_mpv.getMetaPresents();
    m_metapresents.insert(m_metapresents.end(), new_mps.begin(), new_mps.end());
}
/*
void MetaPresentV::sortByDescendingSide(const int side_index){
    struct {
        bool operator()(MetaPresent const &p1, MetaPresent const &p2){
            return p1.getSide(side_index)>p2.getSide(side_index);
        }
    } descsideless;
    std::sort(m_metapresents.begin(), m_metapresents.end(), descsideless);
}*/

struct descsideless{
        bool operator()(MetaPresent const &p1, MetaPresent const &p2){
            return (p1.getSide(0)*p1.getSide(1))>(p2.getSide(0)*p2.getSide(1));
        }
    } ;

void MetaPresentV::sortByDescendingArea(){
    
    std::sort(m_metapresents.begin(), m_metapresents.end(), descsideless());
}

struct maxxyside {
        bool operator()(MetaPresent const &p1, MetaPresent const &p2){
            return (p1.getSide(0)>p1.getSide(1)?p1.getSide(0):p1.getSide(1) + p1.getOrderOffset()) > 
              (p2.getSide(0)>p2.getSide(1)?p2.getSide(0):p2.getSide(1) + p2.getOrderOffset()); 
        }
    } ;

void MetaPresentV::sortByMaxXYSide(const std::vector<double> &oos){
     
    for (int i = 0; i < m_metapresents.size(); i++){
        m_metapresents[i].setOrderOffset(oos[i]);
    }
    std::sort(m_metapresents.begin(), m_metapresents.end(), maxxyside());
}

struct order {
        bool operator()(MetaPresent const &p1, MetaPresent const &p2){
            return p1.getOrder() < p2.getOrder(); ; 
        }
    } ;

void MetaPresentV::sortByOrder(){

    
    std::sort(m_metapresents.begin(), m_metapresents.end(), order());
}


int MetaPresentV::maxHeight(){
    int max_height = 0;
    for (int i = 0; i < m_metapresents.size(); i++){
        if (getMetaPresent(i).getSide(2) > max_height){
            max_height = getMetaPresent(i).getSide(2);
        }
    }
    return max_height;
}
double MetaPresentV::addMetaLevel(){

    std::vector<MetaPresent> to_merge;
    //keep these separate until end so we don't have
    //to worry about present index/order
    
    int max_height = maxHeight();
    int merge_height = maxHeight();
    //todo experiment with other heights
    
    double score_sum = 0;
    //todo these just for tuning, delete later
    
    //go backwards starting from end
    //keep track of height that last present is merged at (it must keep decresing)
    //evaluate merge candidates based on height of end present, efficiency
    //try a bunch of rotations
    sortByOrder();
    int max_low_order = 0;
    std::vector<MetaPresent>::iterator up_it = --m_metapresents.end();
    
    while ( (merge_height > 0) and (up_it->getOrder() > max_low_order)){
        
        double best_score = 0;
        std::vector<MetaPresent>::iterator best_low_it = m_metapresents.begin();
        Rotation best_up_r;
        Rotation best_low_r;
        
        
        //find best present to merge into
        for (auto low_it = m_metapresents.begin(); low_it != up_it; ++low_it){
            //std::cout << low_it->getOrder() << "," << up_it->getOrder() <<";";
            //std::cout << low_it->getSide(2);
            //std::cout << up_it->getSide(2);
            Rotation low_r, up_r;
            double score = getScore(low_r, up_r, low_it, up_it, 
                         max_height, merge_height);
            if (score > best_score){
                best_score = score;
                best_low_it = low_it;
                best_low_r = low_r;
                best_up_r = up_r;
            }
        }
        
        //std::cout << "best score: " << best_score;
        if (best_score> 0){ //todo look at effect of other cutoffs
            
            best_low_it->rotate(best_low_r);
            up_it->rotate(best_up_r);
            
            if ((max_height - up_it->getSide(2)) < merge_height){
                merge_height = max_height - up_it->getSide(2);
            }
            
            //std::cout << "merge height: " << merge_height << std::endl;
            MetaPresent new_merge(*best_low_it, *up_it, 
                                  merge_height+up_it->getSide(2));
            //merge_height here is the new present's max_height
            
            
            if (best_low_it->getOrder() > max_low_order){
                max_low_order = best_low_it->getOrder();
            }
           /* 
            std::cout << "removed: ";
            std::cout << best_low_it->getOrder() << " " << best_low_it->getSide(2) << ", ";
            std::cout  << up_it->getOrder() << " " << up_it->getSide(2)<<",";
            std::cout << "space improvement: " << best_score;
            std::cout << std::endl;
            */
            score_sum += best_score;
            if (best_score > 1){
                score_sum -=10.0;
            }
            //std::cout << up_it->getOrder() << " ";  
            
            up_it = m_metapresents.erase(up_it); 
            m_metapresents.erase(best_low_it);
            up_it -= 2;
            //we have two iterators, so be careful
            //up_it now points to the present previous to the one we erased

            score_sum += addOnMergePresents(new_merge, merge_height, 
                                            max_low_order, up_it, max_height);
            //this updates new_merge, merge_height, max_low_order and up_it                     
            //std::cout << up_it->getOrder() << std::endl;                        
            to_merge.push_back(new_merge);
        } 
        else break;
    }
    
    m_metapresents.insert(m_metapresents.end(), to_merge.begin(), to_merge.end());
    //std::cout << "total area fraction saved: " << score_sum << std::endl;
    
    return score_sum;
}

void MetaPresentV::unravel(){
    std::vector<MetaPresent> unraveled_presents;
    for (int i = 0; i < size(); i++){
        MetaPresent p = getMetaPresent(i);
        if (p.isLeaf()){
            unraveled_presents.push_back(p);
        }else{
        
            Coords c = p.getCoords();
            std::vector<std::shared_ptr<MetaPresent>> base_presents;
            base_presents = p.getBasePointers();
            for (auto it = base_presents.begin(); it != base_presents.end(); ++it){
                MetaPresent bp = **it;
                Coords bc = bp.getCoords();
                Coords overall_c = bc;
                for (int i = 0; i < 2; i++){
                    //we specifically do not set z here!
                    //z is determined during layer insertion,
                    //and this function is called during layer generation
                    overall_c.min[i] += c.min[i]-1;
                    overall_c.max[i] = overall_c.min[i] + bp.getSide(i)-1;
                    
                }
                
                bp.place(overall_c);
                unraveled_presents.push_back(bp);
                
            }
        }
    }
    
    m_metapresents = unraveled_presents;
}

double MetaPresentV::getScore(Rotation &best_low_r, Rotation &best_up_r,
                    const std::vector<MetaPresent>::iterator low_it,
                    const std::vector<MetaPresent>::iterator up_it,
                    const int max_height, const int merge_height){

    double best_score = 0.0;
    
    double original_area = ( (up_it->getSide(0)) * (up_it->getSide(1)) +
                (low_it->getSide(0)) * (low_it->getSide(1)));
        
    //try flat orientations first for cases of tiebreaking
    //if we add support for multiple presents
    //this will cease to be optimal.
    double bonus = 0.0;
    
    for (std::vector<Rotation>::const_iterator urit = m_possible_rots.begin(); urit != m_possible_rots.end(); ++urit){
        up_it->rotate(*urit, false);
        if ((up_it->getSide(2) < 180) and ((up_it ->getSide(1) > 220) or (up_it->getSide(0) > 220)) ){
            bonus = 0.0;
        }else{
            bonus = 0.0;
        }
	int up0 = up_it->getSide(0);
	int up1 = up_it->getSide(1);
	int up2 = up_it->getSide(2);
        for (std::vector<Rotation>::const_iterator lrit = m_possible_rots.begin(); lrit != m_possible_rots.end(); ++lrit){                    
            low_it->rotate(*lrit, false);
 	    int low2 = low_it->getSide(2);
            //check if presents fit vertically
            if ( ((low2+up2) <= max_height) and
                  (low2 <= merge_height) ){
                int low0 = low_it->getSide(0);
		int low1 = low_it->getSide(1); 
                double new_area = (low0 > up0 ? low0 : up0 )*
                                  (low1 > up1 ? low1 : up1);
                double score = (original_area - new_area)*0.001*0.001 + bonus;
                
                if ( (low1 < up1) and 
                     (low0 < up0) ){
                     if ((*lrit)[2] == 2){
                        score *= 1.001;
                     }                     
                }
                /*
                score += score*0.00001*(std::abs(up_it->getSide(2) - low_it->getSide(2)));
                score += score*0.0001*(std::abs(up_it->getOrder() - low_it->getOrder())> 50);
                score += score*0.00001*(std::abs(up_it->getOrder() - low_it->getOrder()));
                */
                //TODO is there a better score method?
                if (score > best_score){
                    
                    best_score = score;
                    best_up_r = *urit;
                    best_low_r = *lrit;
                }
            }
        }
    }
    return best_score;
}

double MetaPresentV::addOnMergePresents(MetaPresent &merge, 
                    int &merge_height, int &max_low_order,
                    std::vector<MetaPresent>::iterator &up_it,
                    int max_height){
    double added_area = 0;
    int up_order = up_it->getOrder()+1;
    
    //try adding on presents on top
    
    
    bool can_add = true;
    while (can_add and up_it->getOrder() > max_low_order){
    //todo: try rotation
        
        if (merge.addChild(*up_it, false)){
            //std::cout << "top inserted " << up_it->getOrder() << " " << (up_it->getSide(0)) * (up_it->getSide(1))*0.001*0.001 << std::endl;
            added_area += (up_it->getSide(0)) * (up_it->getSide(1))*0.001*0.001;
            if (merge_height > (max_height - up_it->getSide(2)) ){
                merge_height = (max_height - up_it->getSide(2));
            };
            up_it = --(m_metapresents.erase(up_it));
        }else{
            can_add = false;
        }
    }
    
    //try adding presents on bottom
    Rotation best_low_r;
    
    while (true){
        std::vector<MetaPresent>::iterator low_it = m_metapresents.begin();
        std::vector<MetaPresent>::iterator best_low_it = m_metapresents.begin();
        double best_score = 0;
        while (low_it->getOrder() < up_it->getOrder() ){ 
            //actually it should go to upit but we can't do that for memory reasons and i'm lazy
            low_it->rotate(m_possible_rots[4]);
            double base_score = low_it->getSide(0)*low_it->getSide(1);
            for (int li = 0; li < 6; li++){
                Rotation low_r = m_possible_rots[li];
                low_it->rotate(low_r);
                if (merge.fitChild(*low_it, true)){
                    double score = base_score*1000 + low_it->getSide(2);   
                    if (score > best_score){
                        best_score = score;
                        best_low_r = low_r;
                        best_low_it = low_it;
                    }
                }
            }
            ++low_it;
        }
        if (best_score == 0){
            break;
        }
        best_low_it->rotate(best_low_r);
        merge.addChild(*best_low_it, true);
        //std::cout << "bottom inserted " << best_low_it->getOrder() << " " << (best_low_it->getSide(0)) * (best_low_it->getSide(1))*0.001*0.001 << std::endl;
        added_area += (best_low_it->getSide(0)) * (best_low_it->getSide(1))*0.001*0.001;            
        max_low_order = best_low_it->getOrder() > max_low_order?
                        best_low_it->getOrder() : max_low_order;
        m_metapresents.erase(best_low_it);
        --up_it;//container size changed
    }
    
    
    
    return added_area;
}
