#ifndef __AC_SESSION_HEADER__
#define __AC_SESSION_HEADER__

#include "capwap_dtls.h"
#include "capwap_event.h"
#include "capwap_lock.h"
#include "ac_soap.h"
#include "ieee80211.h"

/* AC packet */
struct ac_packet {
	int plainbuffer;
	char buffer[0];
};

/* */
struct ac_session_control {
	struct sockaddr_storage localaddress;
	unsigned short count;
};

/* */
#define AC_SESSION_ACTION_CLOSE													0
#define AC_SESSION_ACTION_RESET_WTP												1
#define AC_SESSION_ACTION_ESTABLISHED_SESSION_DATA								2
#define AC_SESSION_ACTION_NOTIFY_EVENT											3
#define AC_SESSION_ACTION_ADDWLAN												4
#define AC_SESSION_ACTION_STATION_CONFIGURATION_IEEE80211_ADD_STATION			5
#define AC_SESSION_ACTION_STATION_CONFIGURATION_IEEE80211_DELETE_STATION		6

#define AC_SESSION_DATA_ACTION_ROAMING_STATION									1
#define AC_SESSION_DATA_ACTION_ASSIGN_BSSID										2
#define AC_SESSION_DATA_ACTION_ADD_STATION_STATUS								3
#define AC_SESSION_DATA_ACTION_DELETE_STATION_STATUS							4

/* */
struct ac_session_action {
	long action;
	long param;
	long length;
	char data[0];
};

/* */
#define NOTIFY_ACTION_CHANGE_STATE								0
#define NOTIFY_ACTION_RECEIVE_REQUEST_CONTROLMESSAGE			1
#define NOTIFY_ACTION_RECEIVE_RESPONSE_CONTROLMESSAGE			1

struct ac_session_notify_event_t {
	char idevent[65];

	int action;
	union {
		unsigned long session_state;
		uint32_t ctrlmsg_type;
	};
};

/* Reset notification */
struct ac_notify_reset_t {
	uint32_t vendor;
	uint8_t name[0];
};

/* Add WLAN notification */
struct ac_notify_addwlan_t {
	uint8_t radioid;
	uint8_t wlanid;
	uint16_t capability;
	uint8_t qos;
	uint8_t authmode;
	uint8_t macmode;
	uint8_t tunnelmode;
	uint8_t suppressssid;
	char ssid[CAPWAP_ADD_WLAN_SSID_LENGTH + 1];
};

/* Station Configuration IEEE802.11 add station notification */
struct ac_notify_station_configuration_ieee8011_add_station {
	uint8_t radioid;
	uint8_t address[MACADDRESS_EUI48_LENGTH];
	uint8_t vlan[CAPWAP_ADDSTATION_VLAN_MAX_LENGTH];

	uint8_t wlanid;
	uint16_t associationid;
	uint16_t capabilities;
	uint8_t supportedratescount;
	uint8_t supportedrates[CAPWAP_STATION_RATES_MAXLENGTH];
};

/* Station Configuration IEEE802.11 delete station notification */
struct ac_notify_station_configuration_ieee8011_delete_station {
	uint8_t radioid;
	uint8_t address[MACADDRESS_EUI48_LENGTH];
};

/* */
struct ac_notify_add_station_status {
	uint8_t radioid;
	uint8_t wlanid;
	uint8_t address[MACADDRESS_EUI48_LENGTH];
	uint16_t statuscode;
};

/* */
struct ac_notify_delete_station_status {
	uint8_t radioid;
	uint8_t address[MACADDRESS_EUI48_LENGTH];
	uint16_t statuscode;
};

/* */
struct ac_session_t;
struct ac_session_data_t;

/* AC sessions data */
struct ac_session_data_t {
	int running;
	pthread_t threadid;
	struct capwap_list_item* itemlist;					/* My itemlist into g_ac.sessionsdata */

	/* Reference */
	long count;
	capwap_event_t changereference;

	/* */
	int enabledtls;
	unsigned short mtu;
	unsigned short fragmentid;
	struct capwap_connection connection;
	struct capwap_dtls dtls;

	struct capwap_timeout* timeout;
	unsigned long idtimercontrol;
	unsigned long idtimerkeepalivedead;

	capwap_event_t waitpacket;
	capwap_lock_t sessionlock;
	struct capwap_list* action;
	struct capwap_list* packets;

	struct capwap_packet_rxmng* rxmngpacket;

	struct ac_session_t* session;
	struct capwap_sessionid_element sessionid;
};

/* AC sessions */
struct ac_session_t {
	int running;
	pthread_t threadid;
	struct capwap_list_item* itemlist;					/* My itemlist into g_ac.sessions */

	/* Reference */
	long count;
	capwap_event_t changereference;

	/* Soap */
	struct ac_http_soap_request* soaprequest;

	/* WLAN Reference */
	struct ac_wlans* wlans;

	/* */
	char* wtpid;
	unsigned long state;
	struct ac_state dfa;

	unsigned short binding;
	struct ac_session_data_t* sessiondata;
	struct capwap_sessionid_element sessionid;

	int teardown;
	unsigned short mtu;
	struct capwap_dtls dtls;
	struct capwap_connection connection;

	struct capwap_timeout* timeout;
	unsigned long idtimercontrol;

	capwap_event_t waitpacket;
	capwap_lock_t sessionlock;
	struct capwap_list* action;
	struct capwap_list* packets;

	struct capwap_list* notifyevent;

	unsigned char localseqnumber;
	unsigned char remoteseqnumber;
	unsigned short fragmentid;
	struct capwap_packet_rxmng* rxmngpacket;
	struct capwap_list* requestfragmentpacket;
	struct capwap_list* responsefragmentpacket;
	unsigned char lastrecvpackethash[16];
	int retransmitcount;
};

/* Session */
void* ac_session_thread(void* param);
void ac_session_send_action(struct ac_session_t* session, long action, long param, const void* data, long length);
void ac_session_teardown(struct ac_session_t* session);
void ac_session_close(struct ac_session_t* session);
void ac_session_release_reference(struct ac_session_t* session);

/* Session data */
void* ac_session_data_thread(void* param);
void ac_session_data_close(struct ac_session_data_t* sessiondata);
void ac_session_data_send_action(struct ac_session_data_t* sessiondata, long action, long param, void* data, long length);
void ac_session_data_release_reference(struct ac_session_data_t* sessiondata);

int ac_session_data_send_data_packet(struct ac_session_data_t* sessiondata, uint8_t radioid, uint8_t wlanid, const uint8_t* data, int length, int leavenativeframe);

/* IEEE802.11 Packet */
void ac_ieee80211_packet(struct ac_session_data_t* sessiondata, uint8_t radioid, const struct ieee80211_header* header, int length);

/* */
int ac_has_sessionid(struct capwap_sessionid_element* sessionid);
int ac_has_wtpid(const char* wtpid);
struct ac_session_t* ac_search_session_from_wtpid(const char* wtpid);

/* */
char* ac_get_printable_wtpid(struct capwap_wtpboarddata_element* wtpboarddata);

/* */
void ac_dfa_change_state(struct ac_session_t* session, int state);

/* */
void ac_get_control_information(struct capwap_list* controllist);

/* */
void ac_free_reference_last_request(struct ac_session_t* session);
void ac_free_reference_last_response(struct ac_session_t* session);

/* */
int ac_msgqueue_init(void);
void ac_msgqueue_free(void);
void ac_msgqueue_notify_closethread(pthread_t threadid);

/* */
int ac_dtls_setup(struct ac_session_t* session);
int ac_dtls_data_setup(struct ac_session_data_t* sessiondata);
void ac_dtls_setup_timeout(struct capwap_timeout* timeout, unsigned long index, void* context, void* param);

/* */
void ac_dfa_retransmition_timeout(struct capwap_timeout* timeout, unsigned long index, void* context, void* param);
void ac_dfa_teardown_timeout(struct capwap_timeout* timeout, unsigned long index, void* context, void* param);

/* */
void ac_dfa_state_join(struct ac_session_t* session, struct capwap_parsed_packet* packet);
void ac_dfa_state_postjoin(struct ac_session_t* session, struct capwap_parsed_packet* packet);
void ac_dfa_state_join_timeout(struct capwap_timeout* timeout, unsigned long index, void* context, void* param);

void ac_dfa_state_configure(struct ac_session_t* session, struct capwap_parsed_packet* packet);

void ac_dfa_state_imagedata(struct ac_session_t* session, struct capwap_parsed_packet* packet);

void ac_dfa_state_datacheck(struct ac_session_t* session, struct capwap_parsed_packet* packet);
void ac_dfa_state_datacheck_to_run(struct ac_session_t* session, struct capwap_parsed_packet* packet);
void ac_dfa_state_datacheck_timeout(struct capwap_timeout* timeout, unsigned long index, void* context, void* param);

void ac_dfa_state_run(struct ac_session_t* session, struct capwap_parsed_packet* packet);

void ac_dfa_state_reset(struct ac_session_t* session, struct capwap_parsed_packet* packet);

void ac_dfa_state_teardown(struct ac_session_t* session);

/* Soap function */
struct ac_soap_response* ac_session_send_soap_request(struct ac_session_t* session, char* method, int numparam, ...);
#define ac_soap_authorizewtpsession(s, wtpid)								ac_session_send_soap_request((s), "authorizeWTPSession", 1, "xs:string", "idwtp", wtpid)
#define ac_soap_joinwtpsession(s, wtpid, joinparam)							ac_session_send_soap_request((s), "joinWTPSession", 2, "xs:string", "idwtp", wtpid, "xs:base64Binary", "join", joinparam)
#define ac_soap_configurestatuswtpsession(s, wtpid, confstatusparam)		ac_session_send_soap_request((s), "configureStatusWTPSession", 2, "xs:string", "idwtp", wtpid, "xs:base64Binary", "confstatus", confstatusparam)
#define ac_soap_changestatewtpsession(s, wtpid, changestateparam)			ac_session_send_soap_request((s), "changeStateWTPSession", 2, "xs:string", "idwtp", wtpid, "xs:base64Binary", "changestate", changestateparam)
#define ac_soap_runningwtpsession(s, wtpid)									ac_session_send_soap_request((s), "runningWTPSession", 1, "xs:string", "idwtp", wtpid)
#define ac_soap_teardownwtpsession(s, wtpid)								ac_session_send_soap_request((s), "teardownWTPSession", 1, "xs:string", "idwtp", wtpid)
#define ac_soap_checkwtpsession(s, wtpid)									ac_session_send_soap_request((s), "checkWTPSession", 1, "xs:string", "idwtp", wtpid)
#define ac_soap_updatebackendevent(s, idevent, status)						ac_session_send_soap_request((s), "updateBackendEvent", 2, "xs:string", "idevent", idevent, "xs:int", "status", status)

#endif /* __AC_SESSION_HEADER__ */
