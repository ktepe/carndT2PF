/*
 * particle_filter.cpp
 *
 *  Created on: Dec 12, 2016
 *      Author: Tiffany Huang
 */

#include <random>
#include <algorithm>
#include <iostream>
#include <numeric>
#include <math.h> 
#include <iostream>
#include <sstream>
#include <string>
#include <iterator>

#include "particle_filter.h"

#define ket_debug 0
#define ket_debug2 0


using namespace std;

void ParticleFilter::init(double x, double y, double theta, double std[]) {
	// TODO: Set the number of particles. Initialize all particles to first position (based on estimates of 
	//   x, y, theta and their uncertainties from GPS) and all weights to 1. 
	// Add random Gaussian noise to each particle.
	// NOTE: Consult particle_filter.h for more information about this method (and others in this file).
	
	default_random_engine gen;
	
	//will be set to std[]
	double std_x=std[0];
	double std_y=std[1];
	double std_psi=std[2];
	
	// TODO: Create normal distributions for y and psi
	normal_distribution<double> dist_x(x, std_x);
	normal_distribution<double> dist_y(y, std_y);
	normal_distribution<double> dist_psi(theta, std_psi);
	
	num_particles=50; 
	//create weights	
	
	//create particles
	//particles.reserve(num_particles);
	
	// initialize particles
	if(!is_initialized){
	
		for (int i=0; i < num_particles; i++) {
			Particle p;
			
			//
			p.id=i;
			p.x = dist_x(gen);
			p.y = dist_y(gen);
			p.theta = dist_psi(gen);
			p.weight=1.0;
			//			
			particles.push_back(p);
#if ket_debug2
			cout << "ParticleFilter::init, particles  i " << particles[i].x << particles[i].y << particles[i].theta << " " << i << endl ;
#endif
		}
	
	  is_initialized=true;
		
	}

} //end of init

void ParticleFilter::prediction(double delta_t, double std_pos[], double velocity, double yaw_rate) {
	// TODO: Add measurements to each particle and add random Gaussian noise.
	// NOTE: When adding noise you may find std::normal_distribution and std::default_random_engine useful.
	//  http://en.cppreference.com/w/cpp/numeric/random/normal_distribution
	//  http://www.cplusplus.com/reference/random/default_random_engine/
#if ket_debug2
	cout<< "ParticleFilter::prediction called" << endl;
#endif

	default_random_engine gen;
	// TODO: Create normal distributions for y and psi
	double x, y, theta;
	
	//will be set to std[]
	double std_x=std_pos[0];
	double std_y=std_pos[1];
	double std_psi=std_pos[2];
	
	normal_distribution<double> dist_x(0, std_x);
	normal_distribution<double> dist_y(0, std_y);
	normal_distribution<double> dist_psi(0, std_psi);
	
	for (int i=0; i < num_particles; i++) {
		x= particles[i].x;
		y= particles[i].y;
		theta=particles[i].theta;
		
		if ( fabs(yaw_rate) > 0.0001) {
			particles[i].x = x + velocity/yaw_rate * ( sin (theta + yaw_rate*delta_t) - sin(theta));
			particles[i].y = y + velocity/yaw_rate * ( cos (theta) -cos(theta+yaw_rate*delta_t)) ;
			particles[i].theta= theta + (yaw_rate * delta_t);
		} else {
		  particles[i].x = x + velocity*delta_t*cos(theta);
      particles[i].y = y + velocity*delta_t*sin(theta);
		}
		//predicted x, y, theta
		x= particles[i].x;
		y= particles[i].y;
		theta=particles[i].theta;
		//add random
		particles[i].x = x+dist_x(gen);
		particles[i].y = y+dist_y(gen);
		particles[i].theta= theta+dist_psi(gen);
#if ket_debug
	cout << "ParticleFilter::prediction, i, particles  " << i<< " x and y "<<particles[i].x << " " <<particles[i].y << "theta " << particles[i].theta <<endl ;
#endif
	}
	
#if ket_debug2
	cout<< "ParticleFilter::prediction ends" << endl;
#endif
}

void ParticleFilter::dataAssociation(std::vector<LandmarkObs> predicted, std::vector<LandmarkObs>& observations) {
	// TODO: Find the predicted measurement that is closest to each observed measurement and assign the 
	//   observed measurement to this particular landmark.
	// NOTE: this method will NOT be called by the grading code. But you will probably find it useful to 
	//   implement this method and use it as a helper during the updateWeights phase.
	
}

void ParticleFilter::updateWeights(double sensor_range, double std_landmark[], 
		std::vector<LandmarkObs> observations, Map map_landmarks) {
	// TODO: Update the weights of each particle using a mult-variate Gaussian distribution. You can read
	//   more about this distribution here: https://en.wikipedia.org/wiki/Multivariate_normal_distribution
	// NOTE: The observations are given in the VEHICLE'S coordinate system. Your particles are located
	//   according to the MAP'S coordinate system. You will need to transform between the two systems.
	//   Keep in mind that this transformation requires both rotation AND translation (but no scaling).
	//   The following is a good resource for the theory:
	//   https://www.willamette.edu/~gorr/classes/GeneralGraphics/Transforms/transforms2d.htm
	//   and the following is a good resource for the actual equation to implement (look at equation 
	//   3.33
	//   http://planning.cs.uiuc.edu/node99.html
	
	//weight.clear();

#if ket_debug2
	cout<< "ParticleFilter::updateWeights called" << endl;
#endif
	
	double sigmax=std_landmark[0];
	double sigmay=std_landmark[1];
	double pi=3.1416; //3.14159265359
	
#if ket_debug	
	cout << "ParticleFilter::updateWeights "<< sensor_range << " " << observations[0].id << " " <<  observations[0].x << " " << observations[0].y << " " << observations.size() << " "<< map_landmarks.landmark_list.size()<<endl;
#endif

#if ket_debug
	for (int i=0; i< observations.size(); i++) {
		cout << "observations "<< i << " " << observations[i].id << " " <<  observations[i].x << " " << observations[i].y << endl;
	}
#endif
	
	for (int j=0; j< num_particles; j++) {	
		//get particles location	
		double px=particles[j].x;
		double py=particles[j].y;
		double ptheta=particles[j].theta;
		double prob=1.0;
		//clear previous associations
		particles[j].associations.clear();
		particles[j].sense_x.clear();
		particles[j].sense_y.clear();
		
		for (int i=0; i < observations.size(); i++) {
			//tranform the observation to global map seen by the particle
			double tx=observations[i].x * cos(ptheta) - observations[i].y * sin(ptheta) ;
			double ty=observations[i].x * sin(ptheta) + observations[i].y * cos(ptheta) ;
		
			//location of observation on the map based on this particle
			double obs_mapx= px+tx;
			double obs_mapy= py+ty;
			
			//we need to find the nearest landmark for this observation		
			double distance_=10000000.0;
			int k_=-1;
			double land_x=0;
			double land_y=0;
			//find the best land_mark
					
			for (int k=0; k<map_landmarks.landmark_list.size(); k++){
				
				double distance_obs_land= dist(obs_mapx, obs_mapy,  map_landmarks.landmark_list[k].x_f,  map_landmarks.landmark_list[k].y_f) ;
				
				if ((distance_obs_land <= distance_) && (distance_obs_land <= sensor_range) ) {
					// update the landmark for this observation of this particle;
					distance_=distance_obs_land	;
					k_=k;
					land_x= map_landmarks.landmark_list[k].x_f;
					land_y= map_landmarks.landmark_list[k].y_f;
				}			
			} //end of matching landmark with observation.
				
			// set association of this particle to land_mark
			//particles[j].associations.push_back(k_);
			//particles[j].sense_x.push_back(obs_mapx);
			//particles[j].sense_y.push_back(obs_mapy);

		
			//now calculate the probability of observation to the landmark
			double gaus_norm=(1.0/(2*pi*sigmax*sigmay));
			double x_dist2=(obs_mapx-land_x)*(obs_mapx-land_x)/(2*sigmax*sigmax);
			double y_dist2=(obs_mapy-land_y)*(obs_mapy-land_y)/(2*sigmay*sigmay);
			
			double probxy=gaus_norm*exp(-(x_dist2+y_dist2));
			
			
			prob=prob*probxy;

		} //obs for loop
		
	
		particles[j].weight=prob;
#if ket_debug		
		cout << "ParticleFilter::updateWeights weights "<< j << "  " << particles[j].weight <<endl;
#endif
		
	}	// particles
	//normalize weights
	/*
	double nor=0.0;
	
	for (int i=0; i< num_particles; i++) nor+=particles[i].weight;
	
	for (int i=0; i< num_particles; i++) {
	
		particles[i].weight=particles[i].weight/nor;
		
#if ket_debug
		cout << "particle weight after normalization " << particles[i].weight << endl;
#endif

	}
	*/
	
#if ket_debug2	
	cout << "ParticleFilter::updateWeights ENDs"<< endl; 
#endif
}

void ParticleFilter::resample() {
	// TODO: Resample particles with replacement with probability proportional to their weight. 
	// NOTE: You may find std::discrete_distribution helpful here.
	//   http://en.cppreference.com/w/cpp/numeric/random/discrete_distribution
	// Vector of weights of all particles
#if ket_debug2
	cout<< "ParticleFilter::resample called" << endl;
#endif
	
	
	default_random_engine gen;
	
	std::vector<double> weights_;
	std::vector<Particle> particles_;
	//create them
	//weights_.reserve(num_particles);
	//particles_.reserve(num_particles);
	
	for(int j = 0; j < num_particles; j++){
        weights_.push_back(particles[j].weight);
    }

	std::discrete_distribution<> d(weights_.begin(), weights_.end());

 
  for(int j = 0; j < num_particles; j++) {
    int index = d(gen);
    particles_.push_back(particles[index]);
  } 

  particles = particles_;

#if ket_debug2
	cout<< "ParticleFilter::resample ends" << endl;
#endif
	
}

Particle ParticleFilter::SetAssociations(Particle particle, std::vector<int> associations, std::vector<double> sense_x, std::vector<double> sense_y)
{
	//particle: the particle to assign each listed association, and association's (x,y) world coordinates mapping to
	// associations: The landmark id that goes along with each listed association
	// sense_x: the associations x mapping already converted to world coordinates
	// sense_y: the associations y mapping already converted to world coordinates

	//Clear the previous associations
	particle.associations.clear();
	particle.sense_x.clear();
	particle.sense_y.clear();

	particle.associations= associations;
 	particle.sense_x = sense_x;
 	particle.sense_y = sense_y;

 	return particle;
}

string ParticleFilter::getAssociations(Particle best)
{
	vector<int> v = best.associations;
	stringstream ss;
    copy( v.begin(), v.end(), ostream_iterator<int>(ss, " "));
    string s = ss.str();
    s = s.substr(0, s.length()-1);  // get rid of the trailing space
    return s;
}
string ParticleFilter::getSenseX(Particle best)
{
	vector<double> v = best.sense_x;
	stringstream ss;
    copy( v.begin(), v.end(), ostream_iterator<float>(ss, " "));
    string s = ss.str();
    s = s.substr(0, s.length()-1);  // get rid of the trailing space
    return s;
}
string ParticleFilter::getSenseY(Particle best)
{
	vector<double> v = best.sense_y;
	stringstream ss;
    copy( v.begin(), v.end(), ostream_iterator<float>(ss, " "));
    string s = ss.str();
    s = s.substr(0, s.length()-1);  // get rid of the trailing space
    return s;
}
