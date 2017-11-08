/**
 *  LEDBERG Flexduino (Connect)
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
definition(
    name: "LEDBERG Flexduino (Connect)",
    namespace: "jamesandariese",
    author: "James Andariese",
    description: "LEDBERG Flexduino (Connect)",
    category: "",
    iconUrl: "https://s3.amazonaws.com/smartapp-icons/Convenience/Cat-Convenience.png",
    iconX2Url: "https://s3.amazonaws.com/smartapp-icons/Convenience/Cat-Convenience@2x.png",
    iconX3Url: "https://s3.amazonaws.com/smartapp-icons/Convenience/Cat-Convenience@2x.png") {
}

preferences {
	page(name: "page1", "title": "Connect", uninstall: true, install: true)
}

def page1() {
	atomicState.done = null
    dynamicPage(name: "page1", "title": "Connect", uninstall: true, install: true) {
		section("Connection") {
			input("name": "ip", type: "text", title: "IP", description: "A static IP is recommended")
        }
	}
}

def handleHubStatusResponseDuringSetup(resp) {
	log.debug resp
    if (resp.json == null || !resp.json.containsKey("istat")) {
    	atomicState.success = false
        atomicState.message = resp.body
    } else {
    	atomicState.status = resp.json
        atomicState.success = true
    }
    atomicState.done = true
}

def installed() {
	log.debug "Installed with settings: ${settings}"

	initialize()
}

def updated() {
	log.debug "Updated with settings: ${settings}"

	unsubscribe()
	initialize()
}

def initialize() {
	// subscribe to http responses.
	subscribe(location, null, locationHandler, [filterEvents:false])
	
    def hasAChild = false
    getChildDevices().each {
    	hasAChild = true
	}
    
    if (!hasAChild) {
	    addChildDevice("LEDBERG Flexduino", "$ip", null, [label: "LEDBERG"])
    }
    runEvery1Minute(poll)
}

def poll() {
	log.debug "Polling"
    
	getChildDevices().each {
    	it.poll()
    }
}

def refreshChild() {
    def params = [
        "headers": [
            "HOST": "$ip:80",
        ],
        "method": "GET",
        "path": "/",
    ]
    log.debug("Refreshing with $params")
    sendHubCommand(new physicalgraph.device.HubAction(params, "$ip:80", [callback: handleHubStatusResponse]))
}

def handleHubStatusResponse(resp) {
	log.debug "handleHubStatusResponse(${resp.body})"
    getChildDevices().each {
    	// only supports one child per app currently so w/e
    	it.update(resp.json)
    }
}

def sendChildHSV(h,s,v) {
    def params = [
        "headers": [
            "HOST": "$ip:80",
        ],
        "method": "GET",
        "path": "/hsv/$h/$s/$v",
    ]
    log.debug("sendChildHSV($h,$s,$v) = $params")
    sendHubCommand(new physicalgraph.device.HubAction(params, "$ip:80", [callback: handleCommandResponse]))
}

def handleCommandResponse(resp) {
	poll()
}