#include "SleighHeights.h"

SleighHeights::SleighHeights(int length, int width, int initial_height){
    m_length = length;
    m_width = width;
    m_max_height = initial_height;
    m_placing_height = initial_height;
}

int SleighHeights::deepest(const Coords &c){
    int max_colliding_height = m_placing_height-1;
    
    for (std::vector<Coords>::iterator it = m_occupied_coords.begin();
         it != m_occupied_coords.end(); ++it){
        
        //is this necessary to keep?
        //TODO this is sorta sketchy since who knows what is calling this
        
        if (it->max[2] <= m_placing_height-1){
            it = m_occupied_coords.erase(it);
            it--;
            continue;
        }
        
        //is this necessary to check?
        if (it->max[2] <= max_colliding_height){
            continue;
        }
        
        //is there an xy collision?
        bool x_c = (c.max[0] >= it->min[0]) and (c.min[0] <= it->max[0]);
        bool y_c = (c.max[1] >= it->min[1]) and (c.min[1] <= it->max[1]);
        if (x_c and y_c){
	    //std::cout << it->min[0] << "," << it->min[1] << std::endl;
            max_colliding_height = it->max[2]; 
            //checked earlier that this would be max height
        }
        
    }
    
    
    return max_colliding_height+1;
}


void SleighHeights::insert(const Coords &c){
    if (c.min[2] > m_placing_height){
        m_placing_height = c.min[2];
    }
    if (c.max[2] > m_max_height){
        m_max_height = c.max[2];
    }
    m_occupied_coords.push_back(c);
    //TODO sometime we need to purge m_occupied_coords!
}
