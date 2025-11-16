/** \copyright
 * Copyright (c) 2025, Jim Kueneman
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  - Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  - Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \file wifi_tools_debug.c
 *
 *
 * @author Jim Kueneman
 * @date 15 Nov 2025
 */




#include "wifi_tools_debug.h"

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define STATUS_LOG_INTERVAL 1000

static uint64_t _status_timer = 0;

void WiFiToolsDebug_log_status(void) {

	if (millis() - _status_timer > STATUS_LOG_INTERVAL) {

		int status = WiFi.status();

		Serial.print("\tstatus: ");
		Serial.print(status);
		Serial.print("\t");
		_status_timer = millis();

		switch (status) {
			case WL_NO_SHIELD:			Serial.print("AP: NO_SHIELD"); 			break; // 255
			case WL_IDLE_STATUS:		Serial.print("AP: IDLE_STATUS"); 		break; // 0
			case WL_NO_SSID_AVAIL:		Serial.print("AP: NO_SSID_AVAIL"); 		break; // 1
			case WL_SCAN_COMPLETED:		Serial.print("AP: SCAN_COMPLETED"); 	break; // 2
			case WL_CONNECTED:			Serial.print("AP: CONNECTED"); 			break; // 3
			case WL_CONNECT_FAILED:		Serial.print("AP: CONNECT_FAILED"); 	break; // 4
			case WL_CONNECTION_LOST:	Serial.print("AP: CONNECTION_LOST"); 	break; // 5
			case WL_DISCONNECTED:		Serial.print("AP: DISCONNECTED"); 		break; // 6		
		}

		Serial.print("\t  rssi: ");
		Serial.println(WiFi.RSSI());
		
	}	
		
}



void WifiToolsDebug_log_event(WiFiEvent_t event, WiFiEventInfo_t info) {

    Serial.print(" --- event: ");
	Serial.print(event);
    Serial.print("\t");

    switch (event) {

        case ARDUINO_EVENT_WIFI_READY:					Serial.println("WIFI_READY"); 				break;	// 0 
		case ARDUINO_EVENT_WIFI_SCAN_DONE:			  	Serial.println("WIFI_SCAN_DONE"); 			break;	// 1 
		case ARDUINO_EVENT_WIFI_STA_START:			  	Serial.println("WIFI_STA_START"); 			break;	// 2 
		case ARDUINO_EVENT_WIFI_STA_STOP:			   	Serial.println("WIFI_STA_STOP");  			break;	// 3 
		case ARDUINO_EVENT_WIFI_STA_CONNECTED:		  	Serial.println("WIFI_STA_CONNECTED"); 		break;	// 4 
		
		case ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE:	Serial.println("WIFI_STA_AUTHMODE_CHANGE");	break;	// 6 
		case ARDUINO_EVENT_WIFI_STA_GOT_IP:	 			Serial.println("WIFI_STA_GOT_IP");			break;	// 7 
		case ARDUINO_EVENT_WIFI_STA_LOST_IP:			Serial.println("WIFI_STA_LOST_IP");   		break;	// 8 


		case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:	   	Serial.print("WIFI_STA_DISCONNECTED");  	// 5 
		
			int reason = info.wifi_sta_disconnected.reason;

			Serial.print(" reason = ");
			Serial.print(reason);
			Serial.print("\t");

			switch (reason) {

				case WIFI_REASON_UNSPECIFIED:                        Serial.println("UNSPECIFIED");				break;
				case WIFI_REASON_AUTH_EXPIRE:                        Serial.println("AUTH_EXPIRE");				break;
				case WIFI_REASON_AUTH_LEAVE:                         Serial.println("AUTH_LEAVE");				break;
				case WIFI_REASON_ASSOC_EXPIRE:                       Serial.println("ASSOC_EXPIRE");			break;
				case WIFI_REASON_ASSOC_TOOMANY:                      Serial.println("ASSOC_TOOMANY");			break;
				case WIFI_REASON_NOT_AUTHED:                         Serial.println("NOT_AUTHED");				break;
				case WIFI_REASON_NOT_ASSOCED:                        Serial.println("NOT_ASSOCED");				break;
				case WIFI_REASON_ASSOC_LEAVE:                        Serial.println("ASSOC_LEAVE");				break;
				case WIFI_REASON_ASSOC_NOT_AUTHED:                   Serial.println("ASSOC_NOT_AUTHED");		break;
				case WIFI_REASON_DISASSOC_PWRCAP_BAD:                Serial.println("DISASSOC_PWRCAP_BAD");		break;
				case WIFI_REASON_DISASSOC_SUPCHAN_BAD:               Serial.println("DISASSOC_SUPCHAN_BAD");	break;
				case WIFI_REASON_BSS_TRANSITION_DISASSOC:            Serial.println("BSS_TRANS_DISASSOC");	break;
				case WIFI_REASON_IE_INVALID:                         Serial.println("IE_INVALID");				break;
				case WIFI_REASON_MIC_FAILURE:                        Serial.println("MIC_FAILURE");				break;
				case WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT:             Serial.println("4WAY_HANDSHAKE_TOUT");	break;
				case WIFI_REASON_GROUP_KEY_UPDATE_TIMEOUT:           Serial.println("GROUP_KEY_UPDATE_TOUT");	break;
				case WIFI_REASON_IE_IN_4WAY_DIFFERS:                 Serial.println("IE_IN_4WAY_DIFFERS");		break;
				case WIFI_REASON_GROUP_CIPHER_INVALID:               Serial.println("GROUP_CIPHER_INVALID");	break;
				case WIFI_REASON_PAIRWISE_CIPHER_INVALID:            Serial.println("PAIRWISE_CIPHER_INVALID");	break;
				case WIFI_REASON_AKMP_INVALID:                       Serial.println("AKMP_INVALID");			break;
				case WIFI_REASON_UNSUPP_RSN_IE_VERSION:              Serial.println("UNSUPP_RSN_IE_VERSION");	break;
				case WIFI_REASON_INVALID_RSN_IE_CAP:                 Serial.println("INVALID_RSN_IE_CAP");		break;
				case WIFI_REASON_802_1X_AUTH_FAILED:                 Serial.println("802_1X_AUTH_FAILED");		break;
				case WIFI_REASON_CIPHER_SUITE_REJECTED:              Serial.println("CIPHER_SUITE_REJECTED");	break;
				case WIFI_REASON_TDLS_PEER_UNREACHABLE:              Serial.println("TDLS_PEER_UNREACHABLE");	break;
				case WIFI_REASON_TDLS_UNSPECIFIED:                   Serial.println("TDLS_UNSPECIFIED");		break;
				case WIFI_REASON_SSP_REQUESTED_DISASSOC:             Serial.println("SSP_REQUESTED_DISASSOC");	break;
				case WIFI_REASON_NO_SSP_ROAMING_AGREEMENT:           Serial.println("NO_SSP_ROAMING_AGRMNT");	break;
				case WIFI_REASON_BAD_CIPHER_OR_AKM:                  Serial.println("BAD_CIPHER_OR_AKM");		break;
				case WIFI_REASON_NOT_AUTHORIZED_THIS_LOCATION:       Serial.println("NOT_AUTHORIZED_THIS_LOC");	break;
				case WIFI_REASON_SERVICE_CHANGE_PERCLUDES_TS:        Serial.println("SVC_CHG_PERCLUDES_TS");	break;
				case WIFI_REASON_UNSPECIFIED_QOS:                    Serial.println("UNSPECIFIED_QOS");			break;
				case WIFI_REASON_NOT_ENOUGH_BANDWIDTH:               Serial.println("NOT_ENOUGH_BANDWIDTH");	break;
				case WIFI_REASON_MISSING_ACKS:                       Serial.println("MISSING_ACKS");			break;
				case WIFI_REASON_EXCEEDED_TXOP:                      Serial.println("EXCEEDED_TXOP");			break;
				case WIFI_REASON_STA_LEAVING:                        Serial.println("STA_LEAVING");				break;
				case WIFI_REASON_END_BA:                             Serial.println("END_BA");					break;
				case WIFI_REASON_UNKNOWN_BA:                         Serial.println("UNKNOWN_BA");				break;
				case WIFI_REASON_TIMEOUT:                            Serial.println("TIMEOUT");					break;
				case WIFI_REASON_PEER_INITIATED:                     Serial.println("PEER_INITIATED");			break;
				case WIFI_REASON_AP_INITIATED:                       Serial.println("AP_INITIATED");			break;
				case WIFI_REASON_INVALID_FT_ACTION_FRAME_COUNT:      Serial.println("INVALID__FRAME_COUNT");	break;
				case WIFI_REASON_INVALID_PMKID:                      Serial.println("INVALID_PMKID");			break;
				case WIFI_REASON_INVALID_MDE:                        Serial.println("INVALID_MDE");				break;
				case WIFI_REASON_INVALID_FTE:                        Serial.println("INVALID_FTE");				break;
				case WIFI_REASON_TRANSMISSION_LINK_ESTABLISH_FAILED: Serial.println("TRANSM_LINK_EST_FAILED");	break;
				case WIFI_REASON_ALTERATIVE_CHANNEL_OCCUPIED:        Serial.println("ALTERATIVE_CH_OCCUPIED");	break;
				case WIFI_REASON_BEACON_TIMEOUT:                     Serial.println("BEACON_TIMEOUT");			break;
				case WIFI_REASON_NO_AP_FOUND:                        Serial.println("NO_AP_FOUND");				break;
				case WIFI_REASON_AUTH_FAIL:                          Serial.println("AUTH_FAIL");				break;
				case WIFI_REASON_ASSOC_FAIL:                         Serial.println("ASSOC_FAIL");				break;
				case WIFI_REASON_HANDSHAKE_TIMEOUT:                  Serial.println("HANDSHAKE_TIMEOUT");		break;
				case WIFI_REASON_CONNECTION_FAIL:                    Serial.println("CONNECTION_FAIL");			break;
				case WIFI_REASON_AP_TSF_RESET:                       Serial.println("AP_TSF_RESET");			break;
				case WIFI_REASON_ROAMING:                            Serial.println("ROAMING");					break;
				case WIFI_REASON_ASSOC_COMEBACK_TIME_TOO_LONG:       Serial.println("ASSOC_CB_TIME_TOO_LONG");	break;

			}

    }

}