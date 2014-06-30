#pragma once


#include <algorithm>
#include <array>
#include <cmath>
#include <fstream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include <iostream>
//still debugging/tuning metapresents

typedef std::vector<int> Rotation;
/* a rotation is encoded in a permutation of [0, 1, 2].
e.g. {0, 1, 2} means x is the smallest side and z is the largest side.
*/

struct Coords {
/*contains the min and max x/y/z of present's placement
Coord.min=Coord.max={-1, -1, -1} if the present is not yet placed
it is also possible to have a present partially placed, e.g. only x/y set
*/
    std::vector<int> min;
    std::vector<int> max;
    void flip(const bool flip_x, const bool flip_y, const Coords &flip_coords);
};

class MetaPresent {
public:
    MetaPresent(const int o, const int a, const int b, const int c, 
                const Rotation &r);
    //create single-present metapresent with order o, sides a/b/c and rotation r
    
    MetaPresent(MetaPresent lower, MetaPresent upper, int merge_height);
    //creates a deeper level metapresent with these two presents
    
    void rotate(const Rotation &r, bool set=true);
    
    int getSide(const int side_index) const 
        {
	if (m_order == 194){
 	   // std::cout << side_index << " " <<m_lwh[side_index] <<"; ";
	}
	return m_lwh[side_index];}
    //get a side length: 0 for x, 1 for y, 2 for z
    
    void place(const Coords &c) {m_coords = c;}
    
    Rotation getRotation() const {return m_rotation;}
    const Coords& getCoords() const {return m_coords;}
    int getOrder() const {return m_order;}
    
    bool isLeaf() const {return m_leaf;}
    std::vector<std::shared_ptr<MetaPresent>> getBasePointers();
    //returns children pointers
    
    bool addChild(MetaPresent child, bool lower);
    bool fitChild(const MetaPresent &child, bool lower);
    
    void setOrderOffset(double o) {m_order_offset = o;} 
    const double getOrderOffset() const{return m_order_offset;} 
private:
    std::vector<std::shared_ptr<MetaPresent>> m_children_ptrs;
    bool m_leaf; //false if this has any children
    double m_order_offset;
    
    int m_split_height; 
    //highest occupied z-coordinate of lower part (in [1, 250] coordinates])
    
    std::vector<Coords> m_lower_coords; //coords of lower presents
    std::vector<Coords> m_upper_coords; //coords of upper presents
    
    int m_lower_y_cursor;
    int m_lower_x_cursor;
    int m_upper_y_cursor;
    int m_upper_x_cursor;
    int m_lower_max_y;
    int m_upper_max_y;

    Rotation m_rotation;
    std::vector<int> m_lwh;
    std::vector<int> m_sorted_lwh;
    int m_order; 
    Coords m_coords;
};

class MetaPresentV{
public:
    MetaPresentV();
    //create an empty metapresent vector
    MetaPresentV(const std::string &filename, const int count);
    //load a metapresent vector from input file
    //uses {0, 1, 2} for starting rotation
    MetaPresentV(const MetaPresentV &parent, const int start_index, const int count);
    //create a metapresent vector from the elements of another present vector
    
    ~MetaPresentV(){};
    
    double volume() const;
    bool collides(const Coords &c) const;
    //checks if c collides with any of presents in self
    
    int size() const 
        {return m_metapresents.size();}
    const MetaPresent& getMetaPresent(const int index) const 
        {return m_metapresents[index];}
    std::vector<MetaPresent> getMetaPresents() const 
        {return m_metapresents;}
    
    void erase(const int index) 
        {m_metapresents.erase(m_metapresents.begin()+index);}
    void insert(const MetaPresent &p) 
        {m_metapresents.push_back(p);}
    void insert(const MetaPresentV &new_pv);
    void rotate(const Rotation r, const int index) 
        {m_metapresents[index].rotate(r);}
    //void sortByDescendingSide(const int side_index);
    void sortByDescendingArea();
    void sortByMaxXYSide(const std::vector<double> &oos);
    void sortByOrder();
    
    int maxHeight();
    
    double addMetaLevel();
    void unravel();
    //will merge metapresents by one level
    //note: currently we support level 0 and 1 only
        
    
private:
    double getScore(Rotation &best_low_r, Rotation &best_up_r,
                    const std::vector<MetaPresent>::iterator low_it,
                    const std::vector<MetaPresent>::iterator up_it,
                    const int max_height, const int merge_height);
    //get merge score;  updates the rotations to reflect best ones
    
    double addOnMergePresents(MetaPresent &merge, 
                    int &merge_height, int &max_low_order,
                    std::vector<MetaPresent>::iterator &up_it,
                    int max_height);
    //add on presents to a newly formed metapresent
    
    //todo memoize collision stuff
    std::vector<MetaPresent> m_metapresents;

    std::vector<Rotation> m_possible_rots;
    
};
