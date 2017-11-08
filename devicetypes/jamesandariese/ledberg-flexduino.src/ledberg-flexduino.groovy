/**
 *  LEDBERG Flexduino
 *
 *  Copyright 2017 James Andariese
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except
 *  in compliance with the License. You may obtain a copy of the License at:
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software distributed under the License is distributed
 *  on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License
 *  for the specific language governing permissions and limitations under the License.
 *
 */
metadata {
	definition (name: "LEDBERG Flexduino", namespace: "jamesandariese", author: "James Andariese") {
		capability "Color Control"
		capability "Light"
		capability "Switch"
		capability "Switch Level"
        capability "Refresh"
	}


	simulator {
		// TODO: define status and reply messages here
	}

	tiles {
		standardTile("switch", "device.switch", width: 2, height: 2, canChangeIcon: true) {
			state "on", label:'${name}', action:"switch.off", icon:"st.switches.light.on", backgroundColor:"#79b821", nextState:"turningOff"
			state "off", label:'${name}', action:"switch.on", icon:"st.switches.light.off", backgroundColor:"#ffffff", nextState:"turningOn"
			state "turningOn", label:'Turning on', action:"switch.off", icon:"st.switches.light.on", backgroundColor:"#79b821", nextState:"turningOff"
			state "turningOff", label:'Turning off', action:"switch.on", icon:"st.switches.light.off", backgroundColor:"#ffffff", nextState:"turningOn"
		}
		standardTile("refresh", "device.switch", inactiveLabel: false, decoration: "flat") {
			state "default", label:"", action:"refresh.refresh", icon:"st.secondary.refresh"
		}
		controlTile("rgbSelector", "device.color", "color", height: 3, width: 3, inactiveLabel: false) {
			state "color", action:"setColor"
		}

		controlTile("levelSliderControl", "device.level", "slider", height: 1, width: 3, inactiveLabel: false, range:"(0..100)") {
			state "level", action:"switch level.setLevel"
		}
		valueTile("level", "device.level", inactiveLabel: false, icon: "st.illuminance.illuminance.light", decoration: "flat") {
			state "level", label: '${currentValue}%'
		}

		main(["switch"])
		details(["switch", "refresh", "level", "levelSliderControl", "rgbSelector", "colorTempSliderControl", "colorTemp"])
	}
}

def initialize() {
	state.hue = 0
    state.lastLevel = 0
    state.level = 0
    state.saturation = 0
}

def sendHSV(hp, sp, vp) {
	log.debug "sendHSV(${hp}, ${sp}, ${vp})"
	// takes ranges from 0 - 100 and translates into 0-360, 0-1, 0-1
	def h = (int) (hp * 3.6)
    def s = sp / 100.0
    def v = vp / 100.0
	def params = [
	    uri: 'http://192.168.86.150',
    	path: "/hsv/${h}/${s}/${v}"
	]
    
    def good = false
    httpGet(params) { resp ->
    	if (isSuccess) {
        	good = true
            sendEvent(name: "level", value: vp)
            sendEvent(name: "saturation", value: sp)
            sendEvent(name: "hue", value: hp)
            sendEvent(name: "switch", value: vp > 0 ? "on" : "off")
        }
	}
    return good
}

// parse events into attributes
def parse(String description) {
	log.debug "Parsing '${description}'"
}

// handle commands
def setHue(percentage) {
	log.debug "Executing 'setHue'"

	state.hue = percentage
    def h = percentage
    def s = state.saturation
    def v = state.level
    
    sendHSV(h,s,v)
}

def setSaturation(percentage) {
	log.debug "Executing 'setSaturation'"
    
	state.saturation = percentage
    def h = state.hue
    def s = percentage
    def v = state.level
    
    sendHSV(h,s,v)
}

def setColor(Map color) {
	log.debug "Executing 'setColor'"
    
    def h = state.hue
    def s = state.saturation
    def v = state.level
    def lastLevel = v

	color.each { key, value ->
		switch (key) {
			case "hue":
				h = value
				break
			case "saturation":
            	s = value
				break
			case "level":
				v = value
				break
		}
	}
    
    state.hue = h
    state.saturation = s
    state.level = v
    if (v == 0) {
    	state.lastLevel = lastLevel
    }

	sendHSV(h,s,v)
}

def off() {
	log.debug "Executing 'off' sdf"
    
    def h = state.hue
    def s = state.saturation
    def v = state.level
    if (v > 0) {
    	state.lastLevel = v
    }
    state.level = 0

    sendHSV(h,s,0)
}

def on() {
	log.debug "Executing 'on' sdf"
    
    def h = state.hue
    def s = state.saturation
    def v = state.lastLevel
    if (v == 0) {
		v = 100
    }
    state.lastLevel = 100
    sendHSV(h,s,v)
}

def setLevel(percentage) {
	log.debug "Executing 'setLevel'"
    
    def h = state.hue
    def s = state.saturation
    def v = state.level
    
    if (percentage == 0) {
    	if (v > 0) {
        	state.lastLevel = v
        }
	}
    
    state.level = percentage
    v = percentage
    
    sendHSV(h,s,v)
}

def poll() {
	refresh()
}

def refresh() {
	def params = [
	    uri: 'http://192.168.86.150',
    	path: "/"
	]
    
    httpGet(params) { resp ->
    	if (isSuccess) {
            sendEvent(name: "level", value: resp.data.value)
            sendEvent(name: "saturation", value: resp.data.saturation)
            sendEvent(name: "hue", value: resp.data.saturation)
            sendEvent(name: "switch", value: resp.data.value > 0 ? "on" : "off")
        }
	}
}