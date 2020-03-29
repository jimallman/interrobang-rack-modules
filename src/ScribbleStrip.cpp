#include "plugin.hpp"
#include "osdialog.h"

struct ScribbleStrip : Module {
	std::string promptText = "Rt-click to edit";
	std::string scribbleText = promptText;
	bool writeTextFromTop = false;

	json_t *dataToJson() override {
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "labelText", json_string(scribbleText.c_str()));
		json_object_set_new(rootJ, "writeTextFromTop", json_boolean(writeTextFromTop));
		return rootJ;
	}
	void dataFromJson(json_t *rootJ) override {
		json_t *labelTextJ = json_object_get(rootJ, "labelText");
		if (labelTextJ) {
			scribbleText = json_string_value(labelTextJ);
		}

		json_t* writeTextFromTopJ = json_object_get(rootJ, "writeTextFromTop");
		if (writeTextFromTopJ && json_is_true(writeTextFromTopJ)) {
			writeTextFromTop = true;
		} else {
			writeTextFromTop = false;  // module default
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
		// if module is not active (we're in the module browser) ,show a demo message
		std::string scrText = module ? module->scribbleText : "Your message here!";
		nvgFontSize(args.vg, 20);
		nvgFontFaceId(args.vg, font->handle);
		nvgTextLetterSpacing(args.vg, 0);
		nvgFillColor(args.vg, nvgRGBA(0xff, 0xff, 0xff, 0xff));
		float lineBreak = 325;
		// test the height of the proposed text, adjust position as needed
		float testBounds[4];
		nvgTextBoxBounds(args.vg, 0, 0, lineBreak, scrText.c_str(), NULL, testBounds);
		// adjust below for one or two lines of text
		bool multilineText = (testBounds[3] > 20);
		float startX, startY;
		// prepare layout based on chosen text orientation
		if (module && module->writeTextFromTop) {
			nvgRotate(args.vg, nvgDegToRad(90.0f));  // runs down!
			startX = 22 - lineBreak;
			startY = multilineText ? -11 : 1;
		} else {
			//nvgRotate(args.vg, -M_PI / 2.0f);  // runs up
			nvgRotate(args.vg, nvgDegToRad(-90.0f));  // runs up?
			startX = -6;
			startY = multilineText ? 1 : 11;
		}
		nvgTextBox(args.vg, startX, startY, lineBreak, scrText.c_str(), NULL);
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

	struct ScribbleEditMenuItem : MenuItem {
		ScribbleStrip *module ;
		void onAction(const event::Action &e) override {
			std::string beforeValue = (module->scribbleText == module->promptText) ? "" : module->scribbleText.c_str();
			char *label = osdialog_prompt(OSDIALOG_INFO, "Label :", beforeValue.c_str());
			if (label) {
				module->scribbleText = std::string(label);
				free(label);
			}
		}
	};
	struct ScribbleFlipMenuItem : MenuItem {
		ScribbleStrip *module ;
		void onAction(const event::Action &e) override {
			module->writeTextFromTop = !(module->writeTextFromTop);
			DEBUG("writeTextFromTop is now %d", module->writeTextFromTop);
		}
	};
	void appendContextMenu(Menu *menu) override {
		//LABEL *module = dynamic_cast<LABEL*>(this->module);
		ScribbleStrip *module = dynamic_cast<ScribbleStrip*>(this->module);
		assert(module);

		menu->addChild(new MenuEntry);
		ScribbleEditMenuItem *menuEdit = new ScribbleEditMenuItem;
		menuEdit->text = "Edit label";
		menuEdit->module = module;
		menu->addChild(menuEdit);
		ScribbleFlipMenuItem *menuFlip = new ScribbleFlipMenuItem;
		menuFlip->text = "Flip text top-to-bottom";
		menuFlip->module = module;
		menu->addChild(menuFlip);
	};
};

Model* modelScribbleStrip = createModel<ScribbleStrip, ScribbleStripWidget>("ScribbleStrip");

/* vim: set autoindent noexpandtab tabstop=4 shiftwidth=4: */
