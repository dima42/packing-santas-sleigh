#include "Sleigh.h"

Sleigh::Sleigh(int length, int width) : m_length(length), m_width(width), 
        m_heights(SleighHeights(length, width, 1)), m_present_volume(0) {
    
    std::random_device r;
    m_gen.seed(r());

    global_gen.seed(global_r());
    global_mut1 = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
}

void Sleigh::packPresents(const MetaPresentV &presents){
    //MetaPresentV to_place = presents;
    int present_index = 0;
    double meta_score_sum = 0;
    while (present_index < presents.size()){
        //add a layer
        int n = present_index<699750?16:1;
	MetaPresentV present_subset(presents, present_index, (presents.size()-present_index)>10000?10000:(presents.size()-present_index));
        
        Layer l = bestLayerAnnealing(present_subset, 0, n);
        //Layer l = bestLayerGradient(presents, present_index);
        
        
        int present_cap = Layer::maxPresentsByArea(present_subset, 0, 
                                                 m_length*m_width*1.02);
        //single run for quick testing
        //Genome g(m_gen, 10000, true);
        //Layer l(presents, present_index, g, m_length, m_width, 0, present_cap);
                
        
        insertLayer(l.getMetaPresents());
        present_index += l.getMetaPresents().size();
        meta_score_sum += l.m_meta_score;
        //if (present_index%5000 < 300){
            //std::cout << present_index << " " << 1.0/(m_heights.maxHeight()*1.0/m_present_volume*m_width*m_length) << " " <<meta_score_sum << std::endl;// << std::endl << std::endl;
            std::ofstream outfile ("temp16withtrpohtoh2lookpartiallookaheadv2.txt");
            int score = 10000.0/(m_heights.maxHeight()*1.0/m_present_volume*m_width*m_length);
            if (outfile.is_open()){ outfile << present_index << " " << score  << "\n";
            }
        //}
        
        //TODO
        //greedily add presents through DBL
        //while height doesn't increase
        //update: this is probably now useless because of metapresents?  
        //could invesitgate
        
    }
}


void Sleigh::generateSubmission(const std::string filename){
    std::ofstream outfile (filename);
    if (outfile.is_open()){
        outfile << "PresentId,x1,y1,z1,x2,y2,z2,x3,y3,z3,x4,y4,z4,"
                << "x5,y5,z5,x6,y6,z6,x7,y7,z7,x8,y8,z8\n";
        for (int i =0; i < m_presents.size(); i++){
            MetaPresent p = m_presents.getMetaPresent(i);
            int xmin = p.getCoords().min[0];
            int xmax = p.getCoords().max[0];
            int ymin = p.getCoords().min[1];
            int ymax = p.getCoords().max[1];
            int zmin = p.getCoords().min[2];
            int zmax = p.getCoords().max[2];
            zmin = m_heights.maxHeight()- p.getCoords().max[2]+1; //flip z
            zmax = m_heights.maxHeight() - p.getCoords().min[2]+1;
            outfile << p.getOrder() << "," 
                    <<xmin << "," <<ymin << "," << zmin << "," 
                    <<xmin << "," <<ymin << "," << zmax << ","
                    <<xmin << "," <<ymax << "," << zmin << ","
                    <<xmin << "," <<ymax << "," << zmax << ","
                    <<xmax << "," <<ymin << "," << zmin << ","
                    <<xmax << "," <<ymin << "," << zmax << ","
                    <<xmax << "," <<ymax << "," << zmin << ","
                    <<xmax << "," <<ymax << "," << zmax << "\n";
        }
        outfile.close();
    }
}

Layer Sleigh::bestLayerGradient(const MetaPresentV &presents, const int start_index){
    
    int present_count = Layer::maxPresentsByArea(presents, start_index, 
                                                 m_length*m_width);
    Genome g(m_gen, present_count, true);                                             
    Layer best_layer(presents, start_index, g, m_length, m_width, 0, present_count);
    int cutoff_count = best_layer.getMetaPresents().size();                                                      
    for (int i = 0; i < present_count; i++){
        Rotation r_alt;
        r_alt.push_back(0);
        r_alt.push_back(1);
        r_alt.push_back(2);
        Genome g_alt = g;
        g_alt.setRotation(r_alt, i);
        Layer l_alt(presents, start_index, g_alt, m_length, m_width, cutoff_count, present_count);
        
        if (l_alt.getMetaPresents().size() > cutoff_count){
            cutoff_count = l_alt.getMetaPresents().size();
            best_layer = l_alt;
            g = g_alt;
        }
    }
    return best_layer;
}

void *annealingThread(void *arg_ptr){
    AnnealingThreadArgs *args_p =  static_cast<AnnealingThreadArgs*>(arg_ptr);
    AnnealingThreadArgs args = *args_p;
    args.new_genome = args.old_genome;
    args.new_genome.mutate(global_gen, args.mutate_rate);
    Layer this_layer(args.presents, args.start_index, args.new_genome, 1000, 1000, args.cutoff_count-args.temp*400, args.present_count);
    args.l = this_layer;
    pthread_mutex_lock(&global_mut1); 
    global_annealing_candidates.push_back(args);
    //std::cout << global_annealing_candidates.size() << std::endl;
    pthread_mutex_unlock(&global_mut1);
}

Layer Sleigh::bestLayerAnnealing(const MetaPresentV &presents, const int start_index, const int steps){
    int cutoff_count = 0;
    int present_count = Layer::maxPresentsByArea(presents, start_index, 
                                                 m_length*m_width*1.04);
    int present_count2 = Layer::maxPresentsByArea(presents, start_index, 
                                                 m_length*m_width*1.00);                                                 
    double temp = 0.01;
    Genome g(m_gen, present_count, false);
    
    Layer best_layer(presents, start_index, g, m_length, m_width, cutoff_count-temp*400, present_count);
    
    //std::uniform_real_distribution<> prob(0.0, 1.0);
    double best_score = best_layer.getMetric();
    double current_score = best_layer.getMetric();
    //std::cout << present_count2 << " ";
    srand (static_cast <unsigned> (time(0)));
    for (int i = 1; i < steps; i++){
        temp -= temp/steps;
        Genome old_g = g;
	
	global_annealing_candidates.clear();
	/*	
	AnnealingThreadArgs test_args;
	test_args.old_genome = old_g;
	test_args.new_genome = g;
	test_args.new_genome.mutate(m_gen, 1.0*(steps-i)/steps);
	Layer l(presents, start_index, test_args.new_genome, m_length, m_width, cutoff_count-temp*400, present_count);
	test_args.l = l;

	global_annealing_candidates.push_back(test_args);
	*/	
	
   	AnnealingThreadArgs new_args;
	new_args.presents = presents;
	Layer l;
	new_args.l = l;
	new_args.old_genome = old_g;
	new_args.new_genome = g;
	new_args.cutoff_count = cutoff_count;
	new_args.present_count = present_count;
	new_args.start_index = start_index;
	new_args.mutate_rate = 1.0*(steps-i+1)/steps;
	new_args.temp = temp;

	void *arg_ptr1 = &new_args;
	pthread_t thread1;
	pthread_create(&thread1, NULL, annealingThread, arg_ptr1);
         
	AnnealingThreadArgs args2 = new_args;
	pthread_t thread2;
        void *arg_ptr2 = &args2;
	pthread_create(&thread2, NULL, annealingThread, arg_ptr2);
	
	AnnealingThreadArgs args3 = new_args;
	pthread_t thread3;
	void *arg_ptr3 = &args3;
	pthread_create(&thread3, NULL, annealingThread, arg_ptr3);
	
	pthread_join(thread1, NULL);
	pthread_join(thread2, NULL);
	pthread_join(thread3, NULL);
	
        //std::cout << global_annealing_candidates.size() << std::endl;	
	for (auto it = global_annealing_candidates.begin(); it != global_annealing_candidates.end(); ++it){
            //std::cout << "helloworld" << std::endl;
	    int count = it->l.getMetaPresents().size();
            double score = it->l.getMetric();
            //std::cout << count << "," << score << " " ;     
            //std::cout << "goodbyworld" << std::endl;
            float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
            if ( std::exp(-(current_score - score)*1.0/(temp+0.000001)) >  r ){ 
                //if count > cutoff_count exp argument is positive, so accepted
                //std::cout << count << "," << score << " " ;  
                current_score = score;
                cutoff_count = count;
	        g = it->new_genome;
                if (score > best_score){
                    best_layer = it->l;
                    best_score = score;
                }
            }
        }
        if (cutoff_count == present_count){
            break;
        }
    }
    //std::cout << present_count2 - best_layer.getMetaPresents().size() << " ";
    return best_layer;
}

Layer Sleigh::bestLayerGenetic(const MetaPresentV &presents, const int start_index,
                          const int population_count, const int generation_count){

    int cutoff_count = 0;
    Layer best_layer;
    int present_count = Layer::maxPresentsByArea(presents, start_index,
                                                 m_length*m_width);
    
    
    Population p(m_gen, present_count, population_count);
    for (int i = 0; i < generation_count; i++){
    
        if (i > 0){ //evolve if not 1st generation
            p.evolve(m_gen);
            //doesn't yet do anything
        }
    
        //determine score for all genomes
        std::vector<Genome> genomes = p.getGenomes();        
        for (unsigned int i = 0; i < genomes.size(); i++){
            Layer l(presents, start_index, genomes[i], m_length, 
                    m_width, cutoff_count, present_count);
            SleighHeights tmpheights = m_heights;
            
            int count = l.getMetaPresents().size();
            if (count > cutoff_count){
                cutoff_count = count;
                best_layer = l;
            }
            p.setScore(cutoff_count, i);
            
        }
    }
    //std::cout << present_count << " " << cutoff_count << " ";
    
    return best_layer;
}

void Sleigh::insertLayer(const MetaPresentV &layer_presents){
    
    for (unsigned int i = 0; i < layer_presents.size(); i++){
        MetaPresent p = layer_presents.getMetaPresent(i);                
        Coords c = p.getCoords();
        int min_height = m_heights.deepest(c);
	//std::cout << p.getOrder() << " " << min_height << std::endl;
        c.min[2] = min_height;
        c.max[2] = min_height+p.getSide(2)-1;
        m_present_volume += p.getSide(0)*p.getSide(1)*p.getSide(2);
        m_heights.insert(c);
        p.place(c);
        m_presents.insert(p);
    }
}
