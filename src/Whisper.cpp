#include "plugin.hpp"


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
	enum LightIds {
		NUM_LIGHTS
	};

	Whisper() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(DELAY_PARAM, 0.f, 1.f, 0.f, "");
		configParam(WETDRYMIX_PARAM, 0.f, 1.f, 0.f, "");
	}

	void process(const ProcessArgs& args) override {
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