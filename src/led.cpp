#include "led.h"


Led::Led(Log& rlog) : logger(rlog, "[LED]") {
    this -> messageCode = ERROR_NO_ERROR;
    this -> led = new JLed (LED_BUILTIN);
}

void Led::setup () {
    
}

void Led::loop() {            
    if (this -> messageCode != ERROR_NO_ERROR) {
        // Need update to "display" led pattern                
        this -> led -> Update();
        
        if (millis() - lastrun > 5000) {
            if (!this -> led->IsRunning()) {                    
                this->led -> Reset();                    
                lastrun = millis();
            }
        }
    }
    
}

// Message code can be a negative number.
// If negative that means this message is removed
void Led::setMessage(int messageCode) {            
    // Prevent the continuous triggers
    if (this -> messageCode != messageCode) {
        logger << "Incoming error message: " << (String)messageCode;
        this -> messageCode = messageCode;
        if (this->messageCode != ERROR_NO_ERROR) {
            this -> led -> Breathe(300).Repeat(this -> messageCode);
            lastrun = millis();
        } else {
            if (this -> led -> IsRunning()) {
                this -> led-> Stop();
                this -> led-> Reset();
            }
        }
    }            
}
