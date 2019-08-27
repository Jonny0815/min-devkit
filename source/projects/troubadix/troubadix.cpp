/// @file
///	@copyright	Copyright 2018 The Min-DevKit Authors. All rights reserved.
///	@license	Use of this source code is governed by the MIT License found in the License.md file.

#include "c74_min.h"
#include "c74_min_catch.h"

#include <vector>
#include <thread>
#include <shared_mutex>


using namespace c74::min;

class bar {
public:
	bar(vector<audio_bundle*>* audio_h) {

		audio = audio_h;
	};

	friend bool operator==(const bar& b1, const bar& b2);

private:
	vector<audio_bundle*>* audio;
};


class loop {
public:
	loop(vector<bar*> bars_h, int length_h) {

		for (size_t i = length_h; i == 0; i--) {
			bars.push_back(bars_h.back() - i);
		}
	}

	vector<bar*> get_bars() {
		return bars;
	}


private:
	vector<bar*> bars;
};


bool operator==(const bar& b1, const bar& b2) {

	size_t size = 0;
	int    diff = 0;
	if (b1.audio->size() < b2.audio->size()) {
		size = b1.audio->size();
	}
	else {
		size = b2.audio->size();
	}

	for (size_t i = 0; i < size; i++) {

		int frame_count = 0;

		if (b1.audio->at(i)->frame_count() < b2.audio->at(i)->frame_count()) {
			frame_count = b1.audio->at(i)->frame_count();
		}
		else {
			frame_count = b2.audio->at(i)->frame_count();
		}
		for (size_t j = 0; j < frame_count; j++) {

			diff += b1.audio->at(i)->samples()[0][j] - b2.audio->at(i)->samples()[0][j];
		}
	}

	if (diff == 0) {
		return true;
	}
	else {
		return false;
	}
}


class troubadix : public object<troubadix>, public mc_operator<> {
public:
	MIN_DESCRIPTION {"troubadix parameter mapping using advanced algorythmns"};
	MIN_TAGS {"troubadix"};
	MIN_AUTHOR {"Johannes Hitzinger"};
	MIN_RELATED {""};

	inlet<> m_inlet1 {this, "(signal) input 1"};
	inlet<> m_inlet2 {this, "(signal) input 2"};

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



	for (size_t i = 0; i < bar_buffer[!bar_buffer_select]->size(); i++) {    // check if bar is not full of empty audio_bundles
		for (size_t j = 0; j < bar_buffer[!bar_buffer_select]->at(i)->frame_count(); j++) {

			
			if (abs(bar_buffer[!bar_buffer_select]->at(i)->samples(0)[j] * 10000) > 1) {    // multiply by 10000 is important, otherwise everything is below 1, now only mute tracks should be below 1
				current_bars.push_back(new bar(bar_buffer[!bar_buffer_select]));
				cout << "sample value: " << abs(bar_buffer[!bar_buffer_select]->at(i)->samples(0)[j] * 10000) << endl;
				if (current_bars.size() > 128) {

					delete current_bars.front();
				}

					cout << "pushed bar (length " << bar_buffer[!bar_buffer_select]->size() << ") into bars" << endl;
					bar_buffer[!bar_buffer_select] = new vector<audio_bundle*>;

					return {};

				}
			}
			
			
		}

		//doesnt work down here, TODO, dkw

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

	for (size_t i = 0; i < current_bars.size(); i++) {

		delete current_bars.at(i);
	}

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
bool                 bar_buffer_select = 0;    // flip flop for bar_buffer
vector<bar*>          current_bars;


// detected loops
vector<loop*> loops;          // all loops
loop*         active_loop = NULL;    // the loop thats currently playing

std::shared_mutex bar_lock;
// std::thread* t_loopcreate;

void loopcreate() {

	if (current_bars.size() < 2) {    // current_bars needs 2 bar, no comparison otherwise
		return;
	}

	bool found_loop = false;

	for (size_t i = 1; i <= 16; (i * 2)) {    // doubleing bar size at each iteration

		cout << "debug i*2 in loopdetection: " << i << endl;

			if (i <= current_bars.size()) {

			for (size_t j = 0; j < i; j++) {
				if (current_bars.back() - j
					== current_bars.back() - i
						- j) {    // comparing last bar with last bar - loopsize, iterating possible loop length via j
					found_loop = true;
				}
				else {
					found_loop = false;
				}
			}

			if (found_loop) {    // if no brain damage on my side: loop found with lenght between 1 and 16

				active_loop = new loop(current_bars, i);

				loop_lookup(active_loop);
			}
		}
		else {
			return;
		}
	}
}

void loop_lookup(loop* loop_h) {

	// TODO look for same loop ins loops
}
}
;

MIN_EXTERNAL(troubadix);
