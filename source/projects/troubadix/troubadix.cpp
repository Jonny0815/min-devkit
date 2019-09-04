/// @file
///	@copyright	Copyright 2018 The Min-DevKit Authors. All rights reserved.
///	@license	Use of this source code is governed by the MIT License found in the License.md file.

#include "c74_min.h"
#include "c74_min_catch.h"

#include <vector>
#include <thread>
#include <shared_mutex>

#include <iostream>
#include <fstream>

using namespace c74::min;

class bar {
public:
	bar(vector<audio_bundle*>* audio_h) {

		audio = audio_h;
	};

	~bar() {
		for (size_t i = 0; i < audio->size(); i++) {

			delete audio->at(i);

		}

		delete audio;

	}


	vector<audio_bundle*>* get_audio() {
		return audio;
	}

private:
	vector<audio_bundle*>* audio;
};


class loop {
public:
	loop(vector<bar*>* bars_h) {

		bars = bars_h;
	}

	~loop() {

		for (size_t i = 0; i < bars->size(); i++) {

			delete bars->at(i);

		}

		delete bars;
	}

	vector<bar*>* get_bars() {

		return bars;
	}


private:
	vector<bar*>* bars;
};




class troubadix : public object<troubadix>, public mc_operator<> {
public:
	MIN_DESCRIPTION {"troubadix parameter mapping using advanced algorythmns"};
	MIN_TAGS {"troubadix"};
	MIN_AUTHOR {"Johannes Hitzinger"};
	MIN_RELATED {""};

	inlet<> m_inlet1 {this, "(signal) input 1"};
	// inlet<> m_inlet2 {this, "(signal) input 2"};

	/*outlet<> m_outlet1 {
		this,
		"(signal) out 1",
		"signal",
	};
	outlet<> m_outlet2 {
		this,
		"(signal) out 2",
		"signal",
	};*/

	message<> bang {this, "bang", "bar",    // pushing a new bar into bars every time a bar ends (messaged by bang in m4l)
		MIN_FUNCTION {


			bar_lock.lock_shared();
	bar_buffer_select = !bar_buffer_select;

	bar_lock.unlock_shared();


	int loud_freq = 0;

	for (size_t i = 0; i < bar_buffer[!bar_buffer_select]->size(); i++) {    // check if bar is not full of empty audio_bundles
		for (size_t j = 0; j < bar_buffer[!bar_buffer_select]->at(i)->frame_count(); j++) {


			if ((abs(bar_buffer[!bar_buffer_select]->at(i)->samples(0)[j]) * 10000 > 1)
				&& (bar_buffer[!bar_buffer_select]->size() < 3000)) {    // multiply by 10000 is important, otherwise everything is below 1,
																		 // now only mute tracks should be below 1
				loud_freq++;
				cout << "loud freq: " << loud_freq << "i + j" << i << " " << j << endl;
			}    // bar_buffer size over 3000 isnt that realistic, 60 pbm is about 2700, if you want to go slower tweak here

			if (loud_freq > 1000) {

				current_bars.push_back(new bar(bar_buffer[!bar_buffer_select]));
				cout << "pushed bar (length " << bar_buffer[!bar_buffer_select]->size() << ") into bars" << endl;


				cout << "sample value: " << abs(bar_buffer[!bar_buffer_select]->at(i)->samples(0)[j] * 10000) << endl;
				/*if (current_bars.size() > 16) {
				 //TODO: reduce this sometime again
					delete current_bars.front();
				}*/


				bar_buffer[!bar_buffer_select] = new vector<audio_bundle*>;

				loopcreate();

				return {};
			}
		}
	}


	// doesnt work down here, TODO, dkw

	delete bar_buffer[!bar_buffer_select];
	cout << "bar was empty, deleted" << endl;
	bar_buffer[!bar_buffer_select] = new vector<audio_bundle*>;

	return {};
}
}
;


troubadix() {

	bar_buffer[0] = new vector<audio_bundle*>;
	bar_buffer[1] = new vector<audio_bundle*>;


	// t_loopcreate = new std::thread(&loopcreate);
}

~troubadix() {

	// delete t_loopcreate;

	/*std::ofstream savefile;

	savefile.open("save.txt");

	for (size_t i = 0; i < current_bars.size(); i++) {

		for (size_t j = 0; j < current_bars.at(i)->get_audio()->size(); j++) {

			for (size_t k = 0; k < current_bars.at(i)->get_audio()->at(j)->frame_count(); k++) {


				savefile << std::fixed << current_bars.at(i)->get_audio()->at(j)->samples(0)[k] * 1000 << "_";
			}

			savefile << "\n";

			delete current_bars.at(i)->get_audio()->at(j)->samples(0);
		}

		savefile << "bar size:" << current_bars.at(i)->get_audio()->size() << " \r\n";

		delete current_bars.at(i);
	}

	savefile.close();*/

	// TODO: save all loops
}

void operator()(audio_bundle input, audio_bundle output) {    // push every audio frame into bar

	bar_lock.lock_shared();

	bar_buffer[bar_buffer_select]->push_back(new audio_bundle(input));

	bar_lock.unlock_shared();
}

private:
// collecting played audio bars
vector<audio_bundle*>* bar_buffer[2];    // two bar buffers, one to write to and the other one to compare to ceck before pushing to bars
bool                   bar_buffer_select = 0;    // flip flop for bar_buffer
vector<bar*>           current_bars;


// detected loops
vector<loop*> loops;                 // all loops
loop*         active_loop = NULL;    // the loop thats currently playing

std::shared_mutex bar_lock;
// std::thread* t_loopcreate;

// void loopcreate() {
//
//	cout << "now trying to create loop" << endl;
//
//	if (current_bars.size() < 2) {    // current_bars needs 2 bar, no comparison otherwise
//
//		cout << "fewer than 2 bars" << endl;
//
//		return;
//	}
//
//	bool found_loop = false;
//
//	for (size_t i = 1; i <= 16; (i * 2)) {    // doubleing bar size at each iteration
//
//		cout << "debug i*2 in loopdetection: " << i << endl;
//
//			if (i <= current_bars.size()) {
//
//			for (size_t j = 0; j < i; j++) {
//
//				if (current_bars.at(current_bars.size() - j)->get_audio()->size() != current_bars.at(current_bars.size() - j -
// i)->get_audio()->size()) {
//
//					delete current_bars.at(current_bars.size() - j);
//
//					if (current_bars.size() < 2) {    // current_bars needs 2 bar, no comparison otherwise
//
//						cout << "fewer than 2 bars" << endl;
//
//						return;
//					}
//
//				}
//
//				if (current_bars.at(current_bars.size() - j) == current_bars.at(current_bars.size() - i - j)) {    // comparing last bar with
//last bar - loopsize, iterating possible loop length via j 					found_loop = true;
//				}
//				else {
//					found_loop = false;
//				}
//			}
//
//			if (found_loop) {    // if no brain damage on my side: loop found with lenght between 1 and 16
//
//				active_loop = new loop(current_bars, i);
//
//				cout << "found loop!" << endl;
//
//				loop_lookup(active_loop);
//			}
//		}
//		else {
//			return;
//		}
//	}
//}

void loopcreate() {

	int   over_boundry = 0;
	float boundry      = 0.02; // max difference between the same freq in a frame in a bar in percent
	float boundry_lc   = 0.15;


	if (current_bars.size() < 2) {

		return;
	}

	for (size_t i = 0; i < (*(current_bars.end() - 1))->get_audio()->size(); i++) {    // compare each freq
		if ((*(current_bars.end() - 2))->get_audio()->size() >= i) {

			for (size_t j = 0; j < current_bars.back()->get_audio()->at(i)->frame_count(); j++) {

				if (abs(current_bars.back()->get_audio()->at(i)->samples(0)[j])
						- abs((*(current_bars.end() - 1))->get_audio()->at(i)->samples(0)[j])
					> abs(current_bars.back()->get_audio()->at(i)->samples(0)[j] * boundry)) {    // when diff is over 2%

					over_boundry++;
				}
			}
		}
	}


	cout << "times over boundry: " << over_boundry << endl;

	if (over_boundry < 100) {    // at 128 bpm were looking at around 70000 comparisons, this tollerance maybe needs adjustment

		if (active_loop == NULL) { // no active loop, set current_bars end to active loop 

			vector<bar*>* tmp_vb = new vector<bar*>;

			tmp_vb->push_back(current_bars.back());

			active_loop = new loop(tmp_vb);

			cout << "set new loop" << endl;

			loops.push_back(active_loop); //TODO: check is there is already a loop that represents this loop in loops !!!

		}
		else {

			cout << "trying to verify the loop" << endl;

			int loop_check_boundry = 0;


		/*	std::ofstream savefile;

	savefile.open("save.txt");

	for (size_t i = 0; i < current_bars.back()->get_audio()->size(); i++) {
		for (size_t j = 0; j < current_bars.back()->get_audio()->at(i)->frame_count(); j++) {
			savefile << std::fixed << current_bars.back()->get_audio()->at(i)->samples(0)[j] * 1000 << " ";
		}
		savefile << "\n";
	}

	savefile << "__________________ \n";

	for (size_t i = 0; i < active_loop->get_bars()->at(0)->get_audio()->size(); i++) {
		for (size_t j = 0; j < active_loop->get_bars()->at(0)->get_audio()->at(i)->frame_count(); j++) {
			savefile << std::fixed << active_loop->get_bars()->at(0)->get_audio()->at(i)->samples(0)[j] * 1000 << " ";
		}
		savefile << "\n";
	}


	savefile.close();*/


			for (size_t i = 0; i < current_bars.back()->get_audio()->size(); i++) {    // compare each freq
				if (active_loop->get_bars()->at(0)->get_audio()->size() >= i) { //at 0 because were only wokrking with 1 bar for each loop atm

					for (size_t j = 0; j < current_bars.back()->get_audio()->at(i)->frame_count(); j++) {

						double diff = abs(current_bars.back()->get_audio()->at(i)->samples(0)[j])
							- abs(active_loop->get_bars()->at(0)->get_audio()->at(i)->samples(0)[j]);

						double loop_boundry = abs(current_bars.back()->get_audio()->at(i)->samples(0)[j] * boundry_lc);

						if ( diff > loop_boundry) {    // when diff is over 2%

							loop_check_boundry++;

							//cout << "loop_check_boundry: " << loop_check_boundry << endl;

						}
					}
				}
			}

			cout << "loop_check_boundry: " << loop_check_boundry << endl;

			int limit = active_loop->get_bars()->at(0)->get_audio()->size() * 64 * boundry_lc; // resolution of each sample is 64 frequencies 

			if (loop_check_boundry > limit) { // seems like the "active_loop" is not the same as the actually playing loop

				active_loop = NULL;

				cout << "not the loop thats currently active -> NULL" << endl;

			}

		}
	}

	return;
}
}
;

MIN_EXTERNAL(troubadix);