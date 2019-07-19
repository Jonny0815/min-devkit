/// @file
///	@ingroup 	minexamples
///	@copyright	Copyright 2018 The Min-DevKit Authors. All rights reserved.
///	@license	Use of this source code is governed by the MIT License found in the License.md file.

#include "c74_min.h"
#include "c74_min_catch.h"

using namespace c74::min;


class troubadix : public object<troubadix>, public mc_operator<> {
public:
	MIN_DESCRIPTION {"Analyze a multi-channel signal. "
					 "[mc.min.info~] provides information about what is happening in the multi-channel signal "
					 "with sample accuracy."};
	MIN_TAGS {"analysis"};
	MIN_AUTHOR {"Timothy Place"};
	MIN_RELATED {"info~, jit.fpsgui~, mc.channelcount~"};

	inlet<>  m_inlet1 {this, "(signal) input 1"};
	inlet<>  m_inlet2 {this, "(signal) input 2"};
	outlet<> m_outlet1 {
		this,
		"(signal) out 1",
		"signal",
	};
	outlet<> m_outlet2 {
		this,
		"(signal) out 2",
		"signal",
	};

	void operator()(audio_bundle input, audio_bundle output) {

		for (auto channel = 0; channel < input.channel_count(); ++channel) {

			for (auto i = 0; i < input.frame_count(); ++i) {

				int tmp_i = input.samples()[channel][i] * 1000;
				cout << tmp_i << "+++";
			}

			cout << endl;
		}

		output = input;
	}
};

MIN_EXTERNAL(troubadix);
