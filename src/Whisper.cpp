#include "plugin.hpp"
#include "samplerate.h"

#define HISTORY_SIZE (1<<21)

struct Whisper : Module {
	enum ParamIds {
		DELAY_PARAM,
		WETDRYMIX_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		INPUTSIGNAL_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUTPUTSIGNAL_OUTPUT,
		NUM_OUTPUTS
	};

    // set up sound buffers (adapted from VCV Delay modue)
    dsp::DoubleRingBuffer<float, HISTORY_SIZE> historyBuffer;
	dsp::DoubleRingBuffer<float, 16> outBuffer;
	SRC_STATE* src;
    //float delay = 0.f;     // 0.0 is a real-time mix; 10.0 delays the wet signal by 1 second
    //float wetDryMix = 0.f; // 0.0 is completely dry, 10.0 is completely wet

	Whisper() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);
		configParam(DELAY_PARAM, 0.f, 1.f, 0.6f, "Time", " s", 1.f / 1e-3, 1e-3);
		configParam(WETDRYMIX_PARAM, 0.f, 1.f, 0.5f, "Mix", "%", 0, 100);

		src = src_new(SRC_SINC_FASTEST, 1, NULL);
		assert(src);
	}

    void process(const ProcessArgs &args) override {
        /* Audio signals are typically +/-5V
         * https://vcvrack.com/manual/VoltageStandards.html
         * ...but theoretically the input could be CV, etc.
         */
        float in = inputs[INPUTSIGNAL_INPUT].getVoltage();
        float dry = in;
        float wet = 0.f;

        // Compute delay time in seconds
		float delay = params[DELAY_PARAM].getValue();
		delay = clamp(delay, 0.f, 1.f);
		delay = 1e-3 * std::pow(10.f / 1e-3, delay); // convert to sec?
		// Number of delay samples
		float index = std::round(delay * args.sampleRate);

        // invert the signal, store/buffer the result
        float inverse = -(in);
		if (!historyBuffer.full()) {
			historyBuffer.push(inverse);
		}


        // How many samples do we need consume to catch up?
		float consume = index - historyBuffer.size();
        if (outBuffer.empty()) {
			double ratio = 1.f;
			if (std::fabs(consume) >= 16.f) {
				// Here's where the delay magic is. Smooth the ratio depending on how divergent we are from the correct delay time.
				ratio = std::pow(10.f, clamp(consume / 10000.f, -1.f, 1.f));
                //DEBUG("WHISPER - process 2a2, smoothed ratio is %f", ratio);
			}
			SRC_DATA srcData;
			srcData.data_in = (const float*) historyBuffer.startData();
			srcData.data_out = (float*) outBuffer.endData();
			srcData.input_frames = std::min((int) historyBuffer.size(), 16);
			srcData.output_frames = outBuffer.capacity();
			srcData.end_of_input = false;
			srcData.src_ratio = ratio;
			src_process(src, &srcData);
			historyBuffer.startIncr(srcData.input_frames_used);

			outBuffer.endIncr(srcData.output_frames_gen);  /* BANG, here we crash */

		}

        // reach into history buffer for delayed sample
        if (!outBuffer.empty()) {
			wet = outBuffer.shift();
		}

        // mix using wet/dry value (from 0 to 1.0)
        float mix = params[WETDRYMIX_PARAM].getValue();
        mix = clamp(mix, 0.f, 1.f);
        //DEBUG("mix ratio is %f", mix);
        float out = crossfade(dry, wet, mix);
        out = clamp(out, -5.f, 5.f);

        // Compute the sine output
        outputs[OUTPUTSIGNAL_OUTPUT].setVoltage(out);
	}
};


struct WhisperWidget : ModuleWidget {
	WhisperWidget(Whisper* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Whisper.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15.24, 46.626)), module, Whisper::DELAY_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15.24, 72.025)), module, Whisper::WETDRYMIX_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.24, 21.228)), module, Whisper::INPUTSIGNAL_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(15.24, 97.424)), module, Whisper::OUTPUTSIGNAL_OUTPUT));
	}
};

Model* modelWhisper = createModel<Whisper, WhisperWidget>("Whisper");

/* vim: set autoindent noexpandtab tabstop=4 shiftwidth=4: */
