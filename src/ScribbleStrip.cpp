#include "plugin.hpp"

struct ScribbleStrip : Module {
    std::string scribbleText = "Rt-click to edit";

    json_t *dataToJson() override {
        json_t *rootJ = json_object();
        json_object_set_new(rootJ, "labelText", json_string(scribbleText.c_str()));
        return rootJ;
    }
    void dataFromJson(json_t *rootJ) override {
        json_t *labelTextJ = json_object_get(rootJ, "labelText");
        if (labelTextJ) {
            scribbleText = json_string_value(labelTextJ);
        }
    }

    void process(const ProcessArgs& args) override {
    }
};


struct ScribbleWidget : TransparentWidget {
	ScribbleStrip *module;
    std::shared_ptr<Font> font;

	ScribbleWidget() {
		font = APP->window->loadFont(asset::plugin(pluginInstance, "res/mad-midnight-marker-font/MadMidnightMarker-na91.ttf"));
        // N.B. nanovg seems to only support .ttf fonts, *not* .otf!
	}

	void draw(const DrawArgs &args) override {
        // show an example message in the module browser
        std::string scrText = module ? module->scribbleText : "Your message here!";
        // truncate displayed text?
		std::string to_display = "";
		for (int i=0; i<20; i++) to_display = to_display + scrText[i];
		nvgFontSize(args.vg, 20);
		nvgFontFaceId(args.vg, font->handle);
		nvgTextLetterSpacing(args.vg, 0);
		nvgFillColor(args.vg, nvgRGBA(0xff, 0xff, 0xff, 0xff));
		nvgRotate(args.vg, -M_PI / 2.0f);
		nvgTextBox(args.vg, 8, 8, 350, to_display.c_str(), NULL);
	}
};



struct ScribbleStripWidget : ModuleWidget {
    ScribbleStripWidget(ScribbleStrip* module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ScribbleStrip.svg")));

        ScribbleWidget *msgDisplay = new ScribbleWidget();
		msgDisplay->box.pos = Vec(18, 333);
		msgDisplay->box.size = Vec(130, 250);
		msgDisplay->module = module;
		addChild(msgDisplay);
    }
};


Model* modelScribbleStrip = createModel<ScribbleStrip, ScribbleStripWidget>("ScribbleStrip");

/* vim: set autoindent noexpandtab tabstop=4 shiftwidth=4: */
