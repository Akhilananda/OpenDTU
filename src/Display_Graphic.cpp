void DisplayGraphicClass::loop()
{
    if (_display_type == DisplayType_t::None) {
        return;
    }

    if ((millis() - _lastDisplayUpdate) > _period) {
        float totalPower = 0;
        float totalYieldDay = 0;
        float totalYieldTotal = 0;
// new for temp display
	float temperature = 0;
// end
        uint8_t isprod = 0;

        for (uint8_t i = 0; i < Hoymiles.getNumInverters(); i++) {
            auto inv = Hoymiles.getInverterByPos(i);
            if (inv == nullptr) {
                continue;
            }

            if (inv->isProducing()) {
                isprod++;
            }
// new for temp display
			for (auto& c : inv->Statistics()->getChannelsByType(TYPE_INV)) {
				temperature = inv->Statistics()->getChannelFieldValue(TYPE_INV,c,FLD_T);
			}
// end
            for (auto& c : inv->Statistics()->getChannelsByType(TYPE_AC)) {
                totalPower += inv->Statistics()->getChannelFieldValue(TYPE_AC, c, FLD_PAC);
                totalYieldDay += inv->Statistics()->getChannelFieldValue(TYPE_AC, c, FLD_YD);
                totalYieldTotal += inv->Statistics()->getChannelFieldValue(TYPE_AC, c, FLD_YT);
            }
        }

        _display->clearBuffer();

        // set Contrast of the Display to raise the lifetime
        _display->setContrast(contrast);

        //=====> Logo and Lighting ==========
        //   pxMovement +x (0 - 6 px)
        uint8_t ex = enableScreensaver ? (_mExtra % 7) : 0;
        if (isprod > 0) {
            _display->drawXBMP(5 + ex, 1, 8, 17, bmp_arrow);
            if (showLogo) {
                _display->drawXBMP(_display->getWidth() - 24 + ex, 2, 16, 16, bmp_logo);
            }
        }
        //<=======================

        //=====> Actual Production ==========
        if ((totalPower > 0) && (isprod > 0)) {
            _display->setPowerSave(false);
            if (totalPower > 999) {
                snprintf(_fmtText, sizeof(_fmtText), "%2.1f kW", (totalPower / 1000));
            } else {
                snprintf(_fmtText, sizeof(_fmtText), "%3.0f W", totalPower);
            }
            printText(_fmtText, 1);
            _previousMillis = millis();
        }
        //<=======================

        //=====> Offline ===========
        else {
            printText("offline", 1);
            // check if it's time to enter power saving mode
            if (millis() - _previousMillis >= (_interval * 2)) {
                _display->setPowerSave(enablePowerSafe);
            }
        }
        //<=======================

        //=====> Today & Total Production =======
        snprintf(_fmtText, sizeof(_fmtText), "today: %4.0f Wh", totalYieldDay);
        printText(_fmtText, 2);

        snprintf(_fmtText, sizeof(_fmtText), "total: %.1f kWh", totalYieldTotal);
        printText(_fmtText, 3);
        //<=======================

        //=====> IP or Date-Time ========
        if (!(_mExtra % 10) && NetworkSettings.localIP()) {
            printText(NetworkSettings.localIP().toString().c_str(), 4);
		} 
// new for temp display
		else { 
			if (_mExtra % 5 ) {
				snprintf(_fmtText, sizeof(_fmtText), "Temperature: %.1f °C", temperature);
				printText(_fmtText,4);
			}
// end
		 	else {
            // Get current time
            time_t now = time(nullptr);
            strftime(_fmtText, sizeof(_fmtText), "%a %d.%m.%Y %H:%M", localtime(&now));
            printText(_fmtText, 4);
        	}
		}
        _display->sendBuffer();

        _dispY = 0;
        _mExtra++;
        _lastDisplayUpdate = millis();
    }
