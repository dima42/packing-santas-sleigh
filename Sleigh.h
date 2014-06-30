#pragma once

#ifdef MAINFILE
    #define EXTERN
#else
    #define EXTERN extern
#endif

#include <cstdlib>
#include <cmath>
#include <ctime>
#include <pthread.h>
#include <random>
#include <string>
#include <iostream>

#include "Layer.h"
#include "MetaPresent.h"
#include "SleighHeights.h"


struct AnnealingThreadArgs{
        Layer l;
        MetaPresentV presents;
        Genome old_genome;
        Genome new_genome;
        int cutoff_count;
        int present_count;
        int start_index;
        double mutate_rate;
        double temp;
    };

EXTERN std::mt19937 global_gen;
EXTERN std::random_device global_r;
EXTERN std::vector<AnnealingThreadArgs> global_annealing_candidates;

EXTERN pthread_mutex_t global_mut1;

void *annealingThread(void *arg_ptr);


class Sleigh {
public:
    Sleigh(int length, int width);
    
    void packPresents(const MetaPresentV &presents);
    //pack a layer, then greedily insert individual presents 
    //without increasing max height.  repeat until out of presents.
    
    void generateSubmission(const std::string filename);
    
    int maxHeight() {return m_heights.maxHeight();}
    
private:
    Layer bestLayerGradient(const MetaPresentV &presents, const int start_index);
    Layer bestLayerAnnealing(const MetaPresentV &presents, const int start_index, const int steps);
    Layer bestLayerGenetic(const MetaPresentV &presents, const int start_index,
                    const int population_count, const int generation_count);
    //genetics is in disrepair and unused                       
    //todo decide whether we want to try genetics again, and if so how
    
    void insertLayer(const MetaPresentV &layer_presents);
    
    std::mt19937 m_gen;
    int m_length, m_width;
    double m_present_volume;
    MetaPresentV m_presents; 
    SleighHeights m_heights;

};
