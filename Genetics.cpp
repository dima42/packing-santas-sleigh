#include "Genetics.h"

Genome::Genome(){

}

Genome::Genome(std::mt19937 &mt, const int present_count, const
               bool fixed_rotation){
    //overly loose upper bound, TODO can make more efficient
    
    //set values for the various genes
    m_fixed_rotation = fixed_rotation;
    for (int i = 0; i < present_count*1.2; i++){
        //extra rotations in case we increase present_cap 
        //thanks to metalevel
        //have all new rotations as default, only change when mutating
        Rotation r = generateRotation(mt, true);
        addRotation(r);
	
	double o = generateOrderOffset(mt, true);
	addOrderOffset(o);
    }
}

void Genome::rotatePresents(MetaPresentV &presents) const{
    //possible TODO: add custom sorting
    //possible TODO: z rotations
    
    for (int i = 0; i < presents.size(); i++){
        if (i < m_rotations.size()){
            presents.rotate(m_rotations[i], i);
        }
        else std::cout<< "error: not enough rotations" << std::endl;
    }
    
}

void Genome::mutate(std::mt19937 &mt, double mutation_rate){
    //std::uniform_real_distribution<> mut(0.0, 1.0);
    
    for (int i = 0; i < m_rotations.size(); i++){    
        float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        if (r < mutation_rate){
            m_rotations[i] = generateRotation(mt, m_fixed_rotation);
        }
    }
    for (int i = 0; i < m_order_offsets.size(); i++){
        float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        if (r < mutation_rate){
            m_order_offsets[i] = generateOrderOffset(mt, m_fixed_rotation);
        }
    }
}

std::pair<Coords, Coords> Genome::rectSplit(const Coords &base_rect, 
                                            const Coords &present_coords) const{
    
    //possible TODO: add genetic decision as to whether to do short axis split
    int new_min_y = present_coords.max[1]+1;
    int new_min_x = present_coords.max[0]+1;
    
    Coords top_rect, right_rect;
    top_rect.min = {base_rect.min[0], new_min_y, base_rect.min[2]};
    right_rect.min = {new_min_x, base_rect.min[1], base_rect.min[2]};
    //shorter axis split
    if ( (base_rect.max[0]-base_rect.min[0]) > 
         (base_rect.max[1]-base_rect.min[1]) ){
        //split into left/right first
        top_rect.max = {new_min_x-1, base_rect.max[1], base_rect.min[2]};
        right_rect.max = base_rect.max;
    }else{
        //split into top/bottom first
        top_rect.max = base_rect.max;
        right_rect.max = {base_rect.max[0], new_min_y-1, base_rect.min[2]};
    }
    
    return std::make_pair(top_rect, right_rect);
    
}

void Genome::insertRect(std::vector<Coords> &free_rects, 
                        const Coords new_rect) const{
    std::vector<Coords>::iterator it = free_rects.begin();
    while( (it != free_rects.end()) and 
           (minRectSide(*it) < minRectSide(new_rect)) ){
        ++it;
    }
    free_rects.insert(it, new_rect);
}         

int Genome::minRectSide(const Coords &c) const{
    int d1 = c.max[0]-c.min[0];
    int d2 = c.max[1]-c.min[1];
    return d1<d2?d1:d2;
}                  

Rotation Genome::generateRotation(std::mt19937 &mt, const bool fixed_rotation){
    Rotation r;
    if (fixed_rotation){
        r.push_back(1);
        r.push_back(0);
        r.push_back(2);
        return r;
    }
    
    //std::uniform_int_distribution<> flip(0, 100);
    bool f = (rand()%3)==0;  
    // 2/3 chance {1, 0, 2}, 1/3 chance {0, 1, 2}
    
    r.push_back(!f);
    r.push_back(f);
    r.push_back(2);
    
    return r;
}         

double Genome::generateOrderOffset(std::mt19937 &mt, const bool zero){
    if (zero){
        return 0.0;
    }
    return 0.0*(0.5- (static_cast<float>(rand()))/(static_cast<float>(RAND_MAX)));
}

Population::Population(std::mt19937 &mt, const int present_count, 
                        const int pop_size) : m_present_count(present_count) {
    for (int i = 0; i < pop_size; i++){
        m_genomes.push_back(Genome(mt, present_count, false));
    }
}
/*
Genome Population::geneCrossover(const Genome &parent1, const Genome &parent2, 
                         std::mt19937 &mt) const {
    
    Genome new_g;
    
    double mutation_rate = 0.02;
    std::uniform_real_distribution<> ur01(0.0, 1.0);
    
    //set values for the various genes
    for (int i = 0; i < parent1.size(); i++){
        Rotation r;
        if (ur01(mt) < mutation_rate){
            
            r = new_g.generateRotation(mt, false);
        }else{
            Rotation r;
            if (ur01(mt)<0.5){
                r = parent1.getRotation(i);
            }else{
                r = parent2.getRotation(i);
            }
        }  
        new_g.addRotation(r);
        
    }
} */                        

void Population::evolve(std::mt19937 &mt){
    //just stub to test 0 evolution cases
}
