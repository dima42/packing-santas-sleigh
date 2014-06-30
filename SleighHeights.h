#pragma once

#include "MetaPresent.h"

#include <iostream>


class SleighHeights{
public:
    SleighHeights(int length, int width, int initial_height);
    
    Coords deepestBottomLeft(const MetaPresent &p) const;
    //finds deepest bottom left coordinates to fit present
    //TODO: this function may or may not be computationally feasible
    
    int deepest(const Coords &c);
    //input: Coords with x/y's filled and z's blank
    //output: minimal allowed z at those x/y
    
    void insert(const Coords &c);
    //updates this based on inserting Coords c (with x/y/z's filled in)
    //the presents should be inserted in order
    
    
    int maxHeight() const{ return m_max_height;}
    int placingHeight() const{ return m_placing_height;}
    
private:
    

    int m_length, m_width;
    int m_max_height;
    int m_placing_height;
    
    std::vector<Coords> m_occupied_coords; 
    //we keep track of occupied space by storing Coords of 
    //recent placed presents
    
};
