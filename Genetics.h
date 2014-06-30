#pragma once

#include <random>
#include <vector>

#include "MetaPresent.h"

class Genome{
public:
    Genome(); 
    //empty/useless genome for placeholders
    
    Genome(std::mt19937 &mt, const int present_count, 
           const bool fixed_rotation);      

    void mutate(std::mt19937 &mt, double mutation_rate);
    
    void rotatePresents(MetaPresentV &presents) const;
    std::pair<Coords, Coords> rectSplit(const Coords &base_rect, 
                                        const Coords &present_coords) const;
    //Warning: this assumes that present is in bottom left corner of base_rect
    
    void insertRect(std::vector<Coords> &free_rects, 
                    const Coords new_rect) const;
    //enforces a custom sorted free_rects order
    
    
    int size() const{return m_rotations.size();}
    void setScore(double score){m_score=score;}
    
    Rotation generateRotation(std::mt19937 &mt, const bool fixed_rotation);
    void addRotation(const Rotation &r) 
        {m_rotations.push_back(r);}
    Rotation getRotation(const int index) const 
        {return m_rotations[index];}
    void setRotation(const Rotation &r, const int index) 
        {m_rotations[index] = r;}

    double generateOrderOffset(std::mt19937 &mt, const bool zero);
    void addOrderOffset(const double o)
	{m_order_offsets.push_back(o);}
    std::vector<double> getOrderOffsets() const
	{return m_order_offsets;}
private: 
    int minRectSide(const Coords &c) const;
    bool m_fixed_rotation;
    
    std::vector<Rotation> m_rotations;
    std::vector<double> m_order_offsets;
    double m_score;
};

class Population{
public:
    Population(std::mt19937 &mt, const int present_count, const int pop_size);
    //fills out a population of genomes
    
    void evolve(std::mt19937 &mt); 
    //evolves to the next generation
    
    void setScore(double score, int g_index) 
        {m_genomes[g_index].setScore(score);}
    std::vector<Genome> getGenomes(){return m_genomes;}
    
private:
    Genome geneCrossover(const Genome &parent1, const Genome &parent2, 
                         std::mt19937 &mt) const; 
    //gets half of genes from parent1, half from parent2, and mutates
    
    std::vector<Genome> m_genomes;
    int m_present_count;
};

