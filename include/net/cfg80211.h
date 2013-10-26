#ifndef __NET_CFG80211_H
#define __NET_CFG80211_H
/*
 * 802.11 device and configuration interface
 *
 * Copyright 2006-2010	Johannes Berg <johannes@sipsolutions.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/netdevice.h>
#include <linux/debugfs.h>
#include <linux/list.h>
#include <linux/bug.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>
#include <linux/nl80211.h>
#include <linux/if_ether.h>
#include <linux/ieee80211.h>
#include <net/regulatory.h>





enum ieee80211_band {
	IEEE80211_BAND_2GHZ = NL80211_BAND_2GHZ,
	IEEE80211_BAND_5GHZ = NL80211_BAND_5GHZ,

	
	IEEE80211_NUM_BANDS
};

enum ieee80211_channel_flags {
	IEEE80211_CHAN_DISABLED		= 1<<0,
	IEEE80211_CHAN_PASSIVE_SCAN	= 1<<1,
	IEEE80211_CHAN_NO_IBSS		= 1<<2,
	IEEE80211_CHAN_RADAR		= 1<<3,
	IEEE80211_CHAN_NO_HT40PLUS	= 1<<4,
	IEEE80211_CHAN_NO_HT40MINUS	= 1<<5,
};

#define IEEE80211_CHAN_NO_HT40 \
	(IEEE80211_CHAN_NO_HT40PLUS | IEEE80211_CHAN_NO_HT40MINUS)

struct ieee80211_channel {
	enum ieee80211_band band;
	u16 center_freq;
	u16 hw_value;
	u32 flags;
	int max_antenna_gain;
	int max_power;
	int max_reg_power;
	bool beacon_found;
	u32 orig_flags;
	int orig_mag, orig_mpwr;
};

enum ieee80211_rate_flags {
	IEEE80211_RATE_SHORT_PREAMBLE	= 1<<0,
	IEEE80211_RATE_MANDATORY_A	= 1<<1,
	IEEE80211_RATE_MANDATORY_B	= 1<<2,
	IEEE80211_RATE_MANDATORY_G	= 1<<3,
	IEEE80211_RATE_ERP_G		= 1<<4,
};

struct ieee80211_rate {
	u32 flags;
	u16 bitrate;
	u16 hw_value, hw_value_short;
};

struct ieee80211_sta_ht_cap {
	u16 cap; 
	bool ht_supported;
	u8 ampdu_factor;
	u8 ampdu_density;
	struct ieee80211_mcs_info mcs;
};

struct ieee80211_supported_band {
	struct ieee80211_channel *channels;
	struct ieee80211_rate *bitrates;
	enum ieee80211_band band;
	int n_channels;
	int n_bitrates;
	struct ieee80211_sta_ht_cap ht_cap;
};



struct vif_params {
       int use_4addr;
};

struct key_params {
	u8 *key;
	u8 *seq;
	int key_len;
	int seq_len;
	u32 cipher;
};

enum survey_info_flags {
	SURVEY_INFO_NOISE_DBM = 1<<0,
	SURVEY_INFO_IN_USE = 1<<1,
	SURVEY_INFO_CHANNEL_TIME = 1<<2,
	SURVEY_INFO_CHANNEL_TIME_BUSY = 1<<3,
	SURVEY_INFO_CHANNEL_TIME_EXT_BUSY = 1<<4,
	SURVEY_INFO_CHANNEL_TIME_RX = 1<<5,
	SURVEY_INFO_CHANNEL_TIME_TX = 1<<6,
};

struct survey_info {
	struct ieee80211_channel *channel;
	u64 channel_time;
	u64 channel_time_busy;
	u64 channel_time_ext_busy;
	u64 channel_time_rx;
	u64 channel_time_tx;
	u32 filled;
	s8 noise;
};

struct cfg80211_crypto_settings {
	u32 wpa_versions;
	u32 cipher_group;
	int n_ciphers_pairwise;
	u32 ciphers_pairwise[NL80211_MAX_NR_CIPHER_SUITES];
	int n_akm_suites;
	u32 akm_suites[NL80211_MAX_NR_AKM_SUITES];
	bool control_port;
	__be16 control_port_ethertype;
	bool control_port_no_encrypt;
};

#ifdef CONFIG_BCMDHD_FW_PATH 
struct beacon_parameters {
	u8 *head, *tail;
	int interval, dtim_period;
	int head_len, tail_len;
	const u8 *ssid;
	size_t ssid_len;
	enum nl80211_hidden_ssid hidden_ssid;
	struct cfg80211_crypto_settings crypto;
	bool privacy;
	enum nl80211_auth_type auth_type;
	const u8 *beacon_ies;
	size_t beacon_ies_len;
	const u8 *proberesp_ies;
	size_t proberesp_ies_len;
	const u8 *assocresp_ies;
	size_t assocresp_ies_len;
};
#endif

struct cfg80211_beacon_data {
	const u8 *head, *tail;
	const u8 *beacon_ies;
	const u8 *proberesp_ies;
	const u8 *assocresp_ies;
	const u8 *probe_resp;

	size_t head_len, tail_len;
	size_t beacon_ies_len;
	size_t proberesp_ies_len;
	size_t assocresp_ies_len;
	size_t probe_resp_len;
};

struct mac_address {
	u8 addr[ETH_ALEN];
};

/**
 * struct cfg80211_acl_data - Access control list data
 *
 * @acl_policy: ACL policy to be applied on the station's
 *	entry specified by mac_addr
 * @n_acl_entries: Number of MAC address entries passed
 * @mac_addrs: List of MAC addresses of stations to be used for ACL
 */
struct cfg80211_acl_data {
	enum nl80211_acl_policy acl_policy;
	int n_acl_entries;

	/* Keep it last */
	struct mac_address mac_addrs[];
};

/**
 * struct cfg80211_ap_settings - AP configuration
 *
 * Used to configure an AP interface.
 *
 * @beacon: beacon data
 * @beacon_interval: beacon interval
 * @dtim_period: DTIM period
 * @ssid: SSID to be used in the BSS (note: may be %NULL if not provided from
 *	user space)
 * @ssid_len: length of @ssid
 * @hidden_ssid: whether to hide the SSID in Beacon/Probe Response frames
 * @crypto: crypto settings
 * @privacy: the BSS uses privacy
 * @auth_type: Authentication type (algorithm)
 * @inactivity_timeout: time in seconds to determine station's inactivity.
 * @acl: ACL configuration used by the drivers which has support for
 *	MAC address based access control
 */
struct cfg80211_ap_settings {
	struct cfg80211_beacon_data beacon;

	int beacon_interval, dtim_period;
	const u8 *ssid;
	size_t ssid_len;
	enum nl80211_hidden_ssid hidden_ssid;
	struct cfg80211_crypto_settings crypto;
	bool privacy;
	enum nl80211_auth_type auth_type;
	int inactivity_timeout;
	const struct cfg80211_acl_data *acl;
};

enum plink_actions {
	PLINK_ACTION_INVALID,
	PLINK_ACTION_OPEN,
	PLINK_ACTION_BLOCK,
};

enum station_parameters_apply_mask {
	STATION_PARAM_APPLY_UAPSD = BIT(0),
};

struct station_parameters {
	u8 *supported_rates;
	struct net_device *vlan;
	u32 sta_flags_mask, sta_flags_set;
	u32 sta_modify_mask;
	int listen_interval;
	u16 aid;
	u8 supported_rates_len;
	u8 plink_action;
	u8 plink_state;
	struct ieee80211_ht_cap *ht_capa;
	u8 uapsd_queues;
	u8 max_sp;
};

enum station_info_flags {
	STATION_INFO_INACTIVE_TIME	= 1<<0,
	STATION_INFO_RX_BYTES		= 1<<1,
	STATION_INFO_TX_BYTES		= 1<<2,
	STATION_INFO_LLID		= 1<<3,
	STATION_INFO_PLID		= 1<<4,
	STATION_INFO_PLINK_STATE	= 1<<5,
	STATION_INFO_SIGNAL		= 1<<6,
	STATION_INFO_TX_BITRATE		= 1<<7,
	STATION_INFO_RX_PACKETS		= 1<<8,
	STATION_INFO_TX_PACKETS		= 1<<9,
	STATION_INFO_TX_RETRIES		= 1<<10,
	STATION_INFO_TX_FAILED		= 1<<11,
	STATION_INFO_RX_DROP_MISC	= 1<<12,
	STATION_INFO_SIGNAL_AVG		= 1<<13,
	STATION_INFO_RX_BITRATE		= 1<<14,
	STATION_INFO_BSS_PARAM          = 1<<15,
	STATION_INFO_CONNECTED_TIME	= 1<<16,
	STATION_INFO_ASSOC_REQ_IES	= 1<<17,
	STATION_INFO_STA_FLAGS		= 1<<18,
	STATION_INFO_BEACON_LOSS_COUNT	= 1<<19
};

enum rate_info_flags {
#ifdef CONFIG_QUALCOMM_WLAN
	RATE_INFO_FLAGS_MCS			= BIT(0),
	RATE_INFO_FLAGS_VHT_MCS			= BIT(1),
	RATE_INFO_FLAGS_40_MHZ_WIDTH		= BIT(2),
	RATE_INFO_FLAGS_80_MHZ_WIDTH		= BIT(3),
	RATE_INFO_FLAGS_80P80_MHZ_WIDTH		= BIT(4),
	RATE_INFO_FLAGS_160_MHZ_WIDTH		= BIT(5),
	RATE_INFO_FLAGS_SHORT_GI		= BIT(6),
	RATE_INFO_FLAGS_60G			= BIT(7),
#else
	RATE_INFO_FLAGS_MCS		= 1<<0,
	RATE_INFO_FLAGS_40_MHZ_WIDTH	= 1<<1,
	RATE_INFO_FLAGS_SHORT_GI	= 1<<2,
#endif
};

struct rate_info {
	u8 flags;
	u8 mcs;
	u16 legacy;
#ifdef CONFIG_QUALCOMM_WLAN
	u8 nss;
#endif
};

enum bss_param_flags {
	BSS_PARAM_FLAGS_CTS_PROT	= 1<<0,
	BSS_PARAM_FLAGS_SHORT_PREAMBLE	= 1<<1,
	BSS_PARAM_FLAGS_SHORT_SLOT_TIME	= 1<<2,
};

struct sta_bss_parameters {
	u8 flags;
	u8 dtim_period;
	u16 beacon_interval;
};

struct station_info {
	u32 filled;
	u32 connected_time;
	u32 inactive_time;
	u32 rx_bytes;
	u32 tx_bytes;
	u16 llid;
	u16 plid;
	u8 plink_state;
	s8 signal;
	s8 signal_avg;
	struct rate_info txrate;
	struct rate_info rxrate;
	u32 rx_packets;
	u32 tx_packets;
	u32 tx_retries;
	u32 tx_failed;
	u32 rx_dropped_misc;
	struct sta_bss_parameters bss_param;
	struct nl80211_sta_flag_update sta_flags;

	int generation;

	const u8 *assoc_req_ies;
	size_t assoc_req_ies_len;
	u32 beacon_loss_count;
};

enum monitor_flags {
	MONITOR_FLAG_FCSFAIL		= 1<<NL80211_MNTR_FLAG_FCSFAIL,
	MONITOR_FLAG_PLCPFAIL		= 1<<NL80211_MNTR_FLAG_PLCPFAIL,
	MONITOR_FLAG_CONTROL		= 1<<NL80211_MNTR_FLAG_CONTROL,
	MONITOR_FLAG_OTHER_BSS		= 1<<NL80211_MNTR_FLAG_OTHER_BSS,
	MONITOR_FLAG_COOK_FRAMES	= 1<<NL80211_MNTR_FLAG_COOK_FRAMES,
};

enum mpath_info_flags {
	MPATH_INFO_FRAME_QLEN		= BIT(0),
	MPATH_INFO_SN			= BIT(1),
	MPATH_INFO_METRIC		= BIT(2),
	MPATH_INFO_EXPTIME		= BIT(3),
	MPATH_INFO_DISCOVERY_TIMEOUT	= BIT(4),
	MPATH_INFO_DISCOVERY_RETRIES	= BIT(5),
	MPATH_INFO_FLAGS		= BIT(6),
};

struct mpath_info {
	u32 filled;
	u32 frame_qlen;
	u32 sn;
	u32 metric;
	u32 exptime;
	u32 discovery_timeout;
	u8 discovery_retries;
	u8 flags;

	int generation;
};

struct bss_parameters {
	int use_cts_prot;
	int use_short_preamble;
	int use_short_slot_time;
	u8 *basic_rates;
	u8 basic_rates_len;
	int ap_isolate;
	int ht_opmode;
};

struct mesh_config {
	
	
	u16 dot11MeshRetryTimeout;
	u16 dot11MeshConfirmTimeout;
	u16 dot11MeshHoldingTimeout;
	u16 dot11MeshMaxPeerLinks;
	u8  dot11MeshMaxRetries;
	u8  dot11MeshTTL;
	
	u8  element_ttl;
	bool auto_open_plinks;
	
	u8  dot11MeshHWMPmaxPREQretries;
	u32 path_refresh_time;
	u16 min_discovery_timeout;
	u32 dot11MeshHWMPactivePathTimeout;
	u16 dot11MeshHWMPpreqMinInterval;
	u16 dot11MeshHWMPperrMinInterval;
	u16 dot11MeshHWMPnetDiameterTraversalTime;
	u8  dot11MeshHWMPRootMode;
	u16 dot11MeshHWMPRannInterval;
	bool  dot11MeshGateAnnouncementProtocol;
	bool dot11MeshForwarding;
	s32 rssi_threshold;
};

struct mesh_setup {
	const u8 *mesh_id;
	u8 mesh_id_len;
	u8  path_sel_proto;
	u8  path_metric;
	const u8 *ie;
	u8 ie_len;
	bool is_authenticated;
	bool is_secure;
	int mcast_rate[IEEE80211_NUM_BANDS];
};

struct ieee80211_txq_params {
	enum nl80211_txq_q queue;
	u16 txop;
	u16 cwmin;
	u16 cwmax;
	u8 aifs;
};

struct wiphy;


struct cfg80211_ssid {
	u8 ssid[IEEE80211_MAX_SSID_LEN];
	u8 ssid_len;
};

#define SCAN_REQUEST_MAGIC	0xdeadbeef  
struct cfg80211_scan_request {
	int magic;  
	struct cfg80211_ssid *ssids;
	int n_ssids;
	u32 n_channels;
	const u8 *ie;
	size_t ie_len;

	u32 rates[IEEE80211_NUM_BANDS];

	
	struct wiphy *wiphy;
	struct net_device *dev;
	bool aborted;
	bool no_cck;

	
	struct ieee80211_channel *channels[0];
};

struct cfg80211_match_set {
	struct cfg80211_ssid ssid;
};

struct cfg80211_sched_scan_request {
	struct cfg80211_ssid *ssids;
	int n_ssids;
	u32 n_channels;
	u32 interval;
	const u8 *ie;
	size_t ie_len;
	struct cfg80211_match_set *match_sets;
	int n_match_sets;

	
	struct wiphy *wiphy;
	struct net_device *dev;

	
	struct ieee80211_channel *channels[0];
};

enum cfg80211_signal_type {
	CFG80211_SIGNAL_TYPE_NONE,
	CFG80211_SIGNAL_TYPE_MBM,
	CFG80211_SIGNAL_TYPE_UNSPEC,
};

struct cfg80211_bss {
	struct ieee80211_channel *channel;

	u8 bssid[ETH_ALEN];
	u64 tsf;
	u16 beacon_interval;
	u16 capability;
	u8 *information_elements;
	size_t len_information_elements;
	u8 *beacon_ies;
	size_t len_beacon_ies;
	u8 *proberesp_ies;
	size_t len_proberesp_ies;

	s32 signal;

	void (*free_priv)(struct cfg80211_bss *bss);
	u8 priv[0] __attribute__((__aligned__(sizeof(void *))));
};

const u8 *ieee80211_bss_get_ie(struct cfg80211_bss *bss, u8 ie);


struct cfg80211_auth_request {
	struct cfg80211_bss *bss;
	const u8 *ie;
	size_t ie_len;
	enum nl80211_auth_type auth_type;
	const u8 *key;
	u8 key_len, key_idx;
};

enum cfg80211_assoc_req_flags {
	ASSOC_REQ_DISABLE_HT		= BIT(0),
};

struct cfg80211_assoc_request {
	struct cfg80211_bss *bss;
	const u8 *ie, *prev_bssid;
	size_t ie_len;
	struct cfg80211_crypto_settings crypto;
	bool use_mfp;
	u32 flags;
	struct ieee80211_ht_cap ht_capa;
	struct ieee80211_ht_cap ht_capa_mask;
};

struct cfg80211_deauth_request {
	const u8 *bssid;
	const u8 *ie;
	size_t ie_len;
	u16 reason_code;
};

struct cfg80211_disassoc_request {
	struct cfg80211_bss *bss;
	const u8 *ie;
	size_t ie_len;
	u16 reason_code;
	bool local_state_change;
};

struct cfg80211_ibss_params {
	u8 *ssid;
	u8 *bssid;
	struct ieee80211_channel *channel;
	enum nl80211_channel_type channel_type;
	u8 *ie;
	u8 ssid_len, ie_len;
	u16 beacon_interval;
	u32 basic_rates;
	bool channel_fixed;
	bool privacy;
	bool control_port;
	int mcast_rate[IEEE80211_NUM_BANDS];
};

struct cfg80211_connect_params {
	struct ieee80211_channel *channel;
	u8 *bssid;
	u8 *ssid;
	size_t ssid_len;
	enum nl80211_auth_type auth_type;
	u8 *ie;
	size_t ie_len;
	bool privacy;
	struct cfg80211_crypto_settings crypto;
	const u8 *key;
	u8 key_len, key_idx;
	u32 flags;
	int bg_scan_period;
	struct ieee80211_ht_cap ht_capa;
	struct ieee80211_ht_cap ht_capa_mask;
};

enum wiphy_params_flags {
	WIPHY_PARAM_RETRY_SHORT		= 1 << 0,
	WIPHY_PARAM_RETRY_LONG		= 1 << 1,
	WIPHY_PARAM_FRAG_THRESHOLD	= 1 << 2,
	WIPHY_PARAM_RTS_THRESHOLD	= 1 << 3,
	WIPHY_PARAM_COVERAGE_CLASS	= 1 << 4,
};

struct cfg80211_bitrate_mask {
	struct {
		u32 legacy;
		u8 mcs[IEEE80211_HT_MCS_MASK_LEN];
	} control[IEEE80211_NUM_BANDS];
};
struct cfg80211_pmksa {
	u8 *bssid;
	u8 *pmkid;
};

struct cfg80211_wowlan_trig_pkt_pattern {
	u8 *mask, *pattern;
	int pattern_len;
};

struct cfg80211_wowlan {
	bool any, disconnect, magic_pkt, gtk_rekey_failure,
	     eap_identity_req, four_way_handshake,
	     rfkill_release;
	struct cfg80211_wowlan_trig_pkt_pattern *patterns;
	int n_patterns;
};

struct cfg80211_gtk_rekey_data {
	u8 kek[NL80211_KEK_LEN];
	u8 kck[NL80211_KCK_LEN];
	u8 replay_ctr[NL80211_REPLAY_CTR_LEN];
};

/**
 * struct cfg80211_update_ft_ies_params - FT IE Information
 *
 * This structure provides information needed to update the fast transition IE
 *
 * @md: The Mobility Domain ID, 2 Octet value
 * @ie: Fast Transition IEs
 * @ie_len: Length of ft_ie in octets
 */
struct cfg80211_update_ft_ies_params {
	u16 md;
	const u8 *ie;
	size_t ie_len;
};

/**
 * struct cfg80211_ops - backend description for wireless configuration
 *
 * This struct is registered by fullmac card drivers and/or wireless stacks
 * in order to handle configuration requests on their interfaces.
 *
 * All callbacks except where otherwise noted should return 0
 * on success or a negative error code.
 *
 * All operations are currently invoked under rtnl for consistency with the
 * wireless extensions but this is subject to reevaluation as soon as this
 * code is used more widely and we have a first user without wext.
 *
 * @suspend: wiphy device needs to be suspended. The variable @wow will
 *	be %NULL or contain the enabled Wake-on-Wireless triggers that are
 *	configured for the device.
 * @resume: wiphy device needs to be resumed
 *
 * @add_virtual_intf: create a new virtual interface with the given name,
 *	must set the struct wireless_dev's iftype. Beware: You must create
 *	the new netdev in the wiphy's network namespace! Returns the netdev,
 *	or an ERR_PTR.
 *
 * @del_virtual_intf: remove the virtual interface determined by ifindex.
 *
 * @change_virtual_intf: change type/configuration of virtual interface,
 *	keep the struct wireless_dev's iftype updated.
 *
 * @add_key: add a key with the given parameters. @mac_addr will be %NULL
 *	when adding a group key.
 *
 * @get_key: get information about the key with the given parameters.
 *	@mac_addr will be %NULL when requesting information for a group
 *	key. All pointers given to the @callback function need not be valid
 *	after it returns. This function should return an error if it is
 *	not possible to retrieve the key, -ENOENT if it doesn't exist.
 *
 * @del_key: remove a key given the @mac_addr (%NULL for a group key)
 *	and @key_index, return -ENOENT if the key doesn't exist.
 *
 * @set_default_key: set the default key on an interface
 *
 * @set_default_mgmt_key: set the default management frame key on an interface
 *
 * @set_rekey_data: give the data necessary for GTK rekeying to the driver
 *
 * @start_ap: Start acting in AP mode defined by the parameters.
 * @change_beacon: Change the beacon parameters for an access point mode
 *	interface. This should reject the call when AP mode wasn't started.
 * @stop_ap: Stop being an AP, including stopping beaconing.
 *
 * @add_station: Add a new station.
 * @del_station: Remove a station; @mac may be NULL to remove all stations.
 * @change_station: Modify a given station. Note that flags changes are not much
 *	validated in cfg80211, in particular the auth/assoc/authorized flags
 *	might come to the driver in invalid combinations -- make sure to check
 *	them, also against the existing state! Also, supported_rates changes are
 *	not checked in station mode -- drivers need to reject (or ignore) them
 *	for anything but TDLS peers.
 * @get_station: get station information for the station identified by @mac
 * @dump_station: dump station callback -- resume dump at index @idx
 *
 * @add_mpath: add a fixed mesh path
 * @del_mpath: delete a given mesh path
 * @change_mpath: change a given mesh path
 * @get_mpath: get a mesh path for the given parameters
 * @dump_mpath: dump mesh path callback -- resume dump at index @idx
 * @join_mesh: join the mesh network with the specified parameters
 * @leave_mesh: leave the current mesh network
 *
 * @get_mesh_config: Get the current mesh configuration
 *
 * @update_mesh_config: Update mesh parameters on a running mesh.
 *	The mask is a bitfield which tells us which parameters to
 *	set, and which to leave alone.
 *
 * @change_bss: Modify parameters for a given BSS.
 *
 * @set_txq_params: Set TX queue parameters
 *
 * @set_channel: Set channel for a given wireless interface. Some devices
 *	may support multi-channel operation (by channel hopping) so cfg80211
 *	doesn't verify much. Note, however, that the passed netdev may be
 *	%NULL as well if the user requested changing the channel for the
 *	device itself, or for a monitor interface.
 * @get_channel: Get the current operating channel, should return %NULL if
 *	there's no single defined operating channel if for example the
 *	device implements channel hopping for multi-channel virtual interfaces.
 *
 * @scan: Request to do a scan. If returning zero, the scan request is given
 *	the driver, and will be valid until passed to cfg80211_scan_done().
 *	For scan results, call cfg80211_inform_bss(); you can call this outside
 *	the scan/scan_done bracket too.
 *
 * @auth: Request to authenticate with the specified peer
 * @assoc: Request to (re)associate with the specified peer
 * @deauth: Request to deauthenticate from the specified peer
 * @disassoc: Request to disassociate from the specified peer
 *
 * @connect: Connect to the ESS with the specified parameters. When connected,
 *	call cfg80211_connect_result() with status code %WLAN_STATUS_SUCCESS.
 *	If the connection fails for some reason, call cfg80211_connect_result()
 *	with the status from the AP.
 * @disconnect: Disconnect from the BSS/ESS.
 *
 * @join_ibss: Join the specified IBSS (or create if necessary). Once done, call
 *	cfg80211_ibss_joined(), also call that function when changing BSSID due
 *	to a merge.
 * @leave_ibss: Leave the IBSS.
 *
 * @set_wiphy_params: Notify that wiphy parameters have changed;
 *	@changed bitfield (see &enum wiphy_params_flags) describes which values
 *	have changed. The actual parameter values are available in
 *	struct wiphy. If returning an error, no value should be changed.
 *
 * @set_tx_power: set the transmit power according to the parameters,
 *	the power passed is in mBm, to get dBm use MBM_TO_DBM().
 * @get_tx_power: store the current TX power into the dbm variable;
 *	return 0 if successful
 *
 * @set_wds_peer: set the WDS peer for a WDS interface
 *
 * @rfkill_poll: polls the hw rfkill line, use cfg80211 reporting
 *	functions to adjust rfkill hw state
 *
 * @dump_survey: get site survey information.
 *
 * @remain_on_channel: Request the driver to remain awake on the specified
 *	channel for the specified duration to complete an off-channel
 *	operation (e.g., public action frame exchange). When the driver is
 *	ready on the requested channel, it must indicate this with an event
 *	notification by calling cfg80211_ready_on_channel().
 * @cancel_remain_on_channel: Cancel an on-going remain-on-channel operation.
 *	This allows the operation to be terminated prior to timeout based on
 *	the duration value.
 * @mgmt_tx: Transmit a management frame.
 * @mgmt_tx_cancel_wait: Cancel the wait time from transmitting a management
 *	frame on another channel
 *
 * @testmode_cmd: run a test mode command
 * @testmode_dump: Implement a test mode dump. The cb->args[2] and up may be
 *	used by the function, but 0 and 1 must not be touched. Additionally,
 *	return error codes other than -ENOBUFS and -ENOENT will terminate the
 *	dump and return to userspace with an error, so be careful. If any data
 *	was passed in from userspace then the data/len arguments will be present
 *	and point to the data contained in %NL80211_ATTR_TESTDATA.
 *
 * @set_bitrate_mask: set the bitrate mask configuration
 *
 * @set_pmksa: Cache a PMKID for a BSSID. This is mostly useful for fullmac
 *	devices running firmwares capable of generating the (re) association
 *	RSN IE. It allows for faster roaming between WPA2 BSSIDs.
 * @del_pmksa: Delete a cached PMKID.
 * @flush_pmksa: Flush all cached PMKIDs.
 * @set_power_mgmt: Configure WLAN power management. A timeout value of -1
 *	allows the driver to adjust the dynamic ps timeout value.
 * @set_cqm_rssi_config: Configure connection quality monitor RSSI threshold.
 * @sched_scan_start: Tell the driver to start a scheduled scan.
 * @sched_scan_stop: Tell the driver to stop an ongoing scheduled
 *	scan.  The driver_initiated flag specifies whether the driver
 *	itself has informed that the scan has stopped.
 *
 * @mgmt_frame_register: Notify driver that a management frame type was
 *	registered. Note that this callback may not sleep, and cannot run
 *	concurrently with itself.
 *
 * @set_antenna: Set antenna configuration (tx_ant, rx_ant) on the device.
 *	Parameters are bitmaps of allowed antennas to use for TX/RX. Drivers may
 *	reject TX/RX mask combinations they cannot support by returning -EINVAL
 *	(also see nl80211.h @NL80211_ATTR_WIPHY_ANTENNA_TX).
 *
 * @get_antenna: Get current antenna configuration from device (tx_ant, rx_ant).
 *
 * @set_ringparam: Set tx and rx ring sizes.
 *
 * @get_ringparam: Get tx and rx ring current and maximum sizes.
 *
 * @tdls_mgmt: Transmit a TDLS management frame.
 * @tdls_oper: Perform a high-level TDLS operation (e.g. TDLS link setup).
 *
 * @probe_client: probe an associated client, must return a cookie that it
 *	later passes to cfg80211_probe_status().
 *
 * @set_noack_map: Set the NoAck Map for the TIDs.
 * @set_mac_acl: Sets MAC address control list in AP and P2P GO mode.
 *	Parameters include ACL policy, an array of MAC address of stations
 *	and the number of MAC addresses. If there is already a list in driver
 *	this new list replaces the existing one. Driver has to clear its ACL
 *	when number of MAC addresses entries is passed as 0. Drivers which
 *	advertise the support for MAC based ACL have to implement this callback.
 *
 */
struct cfg80211_ops {
	int	(*suspend)(struct wiphy *wiphy, struct cfg80211_wowlan *wow);
	int	(*resume)(struct wiphy *wiphy);

	struct net_device * (*add_virtual_intf)(struct wiphy *wiphy,
						char *name,
						enum nl80211_iftype type,
						u32 *flags,
						struct vif_params *params);
	int	(*del_virtual_intf)(struct wiphy *wiphy, struct net_device *dev);
	int	(*change_virtual_intf)(struct wiphy *wiphy,
				       struct net_device *dev,
				       enum nl80211_iftype type, u32 *flags,
				       struct vif_params *params);

	int	(*add_key)(struct wiphy *wiphy, struct net_device *netdev,
			   u8 key_index, bool pairwise, const u8 *mac_addr,
			   struct key_params *params);
	int	(*get_key)(struct wiphy *wiphy, struct net_device *netdev,
			   u8 key_index, bool pairwise, const u8 *mac_addr,
			   void *cookie,
			   void (*callback)(void *cookie, struct key_params*));
	int	(*del_key)(struct wiphy *wiphy, struct net_device *netdev,
			   u8 key_index, bool pairwise, const u8 *mac_addr);
	int	(*set_default_key)(struct wiphy *wiphy,
				   struct net_device *netdev,
				   u8 key_index, bool unicast, bool multicast);
	int	(*set_default_mgmt_key)(struct wiphy *wiphy,
					struct net_device *netdev,
					u8 key_index);

#ifdef CONFIG_BCMDHD_FW_PATH 
	int	(*add_beacon)(struct wiphy *wiphy, struct net_device *dev,
			      struct beacon_parameters *info);
	int	(*set_beacon)(struct wiphy *wiphy, struct net_device *dev,
			      struct beacon_parameters *info);
	int	(*del_beacon)(struct wiphy *wiphy, struct net_device *dev);
#endif

	int	(*start_ap)(struct wiphy *wiphy, struct net_device *dev,
			    struct cfg80211_ap_settings *settings);
	int	(*change_beacon)(struct wiphy *wiphy, struct net_device *dev,
				 struct cfg80211_beacon_data *info);
	int	(*stop_ap)(struct wiphy *wiphy, struct net_device *dev);


	int	(*add_station)(struct wiphy *wiphy, struct net_device *dev,
			       u8 *mac, struct station_parameters *params);
	int	(*del_station)(struct wiphy *wiphy, struct net_device *dev,
			       u8 *mac);
	int	(*change_station)(struct wiphy *wiphy, struct net_device *dev,
				  u8 *mac, struct station_parameters *params);
	int	(*get_station)(struct wiphy *wiphy, struct net_device *dev,
			       u8 *mac, struct station_info *sinfo);
	int	(*dump_station)(struct wiphy *wiphy, struct net_device *dev,
			       int idx, u8 *mac, struct station_info *sinfo);

	int	(*add_mpath)(struct wiphy *wiphy, struct net_device *dev,
			       u8 *dst, u8 *next_hop);
	int	(*del_mpath)(struct wiphy *wiphy, struct net_device *dev,
			       u8 *dst);
	int	(*change_mpath)(struct wiphy *wiphy, struct net_device *dev,
				  u8 *dst, u8 *next_hop);
	int	(*get_mpath)(struct wiphy *wiphy, struct net_device *dev,
			       u8 *dst, u8 *next_hop,
			       struct mpath_info *pinfo);
	int	(*dump_mpath)(struct wiphy *wiphy, struct net_device *dev,
			       int idx, u8 *dst, u8 *next_hop,
			       struct mpath_info *pinfo);
	int	(*get_mesh_config)(struct wiphy *wiphy,
				struct net_device *dev,
				struct mesh_config *conf);
	int	(*update_mesh_config)(struct wiphy *wiphy,
				      struct net_device *dev, u32 mask,
				      const struct mesh_config *nconf);
	int	(*join_mesh)(struct wiphy *wiphy, struct net_device *dev,
			     const struct mesh_config *conf,
			     const struct mesh_setup *setup);
	int	(*leave_mesh)(struct wiphy *wiphy, struct net_device *dev);

	int	(*change_bss)(struct wiphy *wiphy, struct net_device *dev,
			      struct bss_parameters *params);

	int	(*set_txq_params)(struct wiphy *wiphy, struct net_device *dev,
				  struct ieee80211_txq_params *params);

	int	(*set_channel)(struct wiphy *wiphy, struct net_device *dev,
			       struct ieee80211_channel *chan,
			       enum nl80211_channel_type channel_type);

	int	(*scan)(struct wiphy *wiphy, struct net_device *dev,
			struct cfg80211_scan_request *request);

	int	(*auth)(struct wiphy *wiphy, struct net_device *dev,
			struct cfg80211_auth_request *req);
	int	(*assoc)(struct wiphy *wiphy, struct net_device *dev,
			 struct cfg80211_assoc_request *req);
	int	(*deauth)(struct wiphy *wiphy, struct net_device *dev,
			  struct cfg80211_deauth_request *req);
	int	(*disassoc)(struct wiphy *wiphy, struct net_device *dev,
			    struct cfg80211_disassoc_request *req);

	int	(*connect)(struct wiphy *wiphy, struct net_device *dev,
			   struct cfg80211_connect_params *sme);
	int	(*disconnect)(struct wiphy *wiphy, struct net_device *dev,
			      u16 reason_code);

	int	(*join_ibss)(struct wiphy *wiphy, struct net_device *dev,
			     struct cfg80211_ibss_params *params);
	int	(*leave_ibss)(struct wiphy *wiphy, struct net_device *dev);

	int	(*set_wiphy_params)(struct wiphy *wiphy, u32 changed);

	int	(*set_tx_power)(struct wiphy *wiphy,
				enum nl80211_tx_power_setting type, int mbm);
	int	(*get_tx_power)(struct wiphy *wiphy, int *dbm);

	int	(*set_wds_peer)(struct wiphy *wiphy, struct net_device *dev,
				const u8 *addr);

	void	(*rfkill_poll)(struct wiphy *wiphy);

#ifdef CONFIG_NL80211_TESTMODE
	int	(*testmode_cmd)(struct wiphy *wiphy, void *data, int len);
	int	(*testmode_dump)(struct wiphy *wiphy, struct sk_buff *skb,
				 struct netlink_callback *cb,
				 void *data, int len);
#endif

	int	(*set_bitrate_mask)(struct wiphy *wiphy,
				    struct net_device *dev,
				    const u8 *peer,
				    const struct cfg80211_bitrate_mask *mask);

	int	(*dump_survey)(struct wiphy *wiphy, struct net_device *netdev,
			int idx, struct survey_info *info);

	int	(*set_pmksa)(struct wiphy *wiphy, struct net_device *netdev,
			     struct cfg80211_pmksa *pmksa);
	int	(*del_pmksa)(struct wiphy *wiphy, struct net_device *netdev,
			     struct cfg80211_pmksa *pmksa);
	int	(*flush_pmksa)(struct wiphy *wiphy, struct net_device *netdev);

	int	(*remain_on_channel)(struct wiphy *wiphy,
				     struct net_device *dev,
				     struct ieee80211_channel *chan,
				     enum nl80211_channel_type channel_type,
				     unsigned int duration,
				     u64 *cookie);
	int	(*cancel_remain_on_channel)(struct wiphy *wiphy,
					    struct net_device *dev,
					    u64 cookie);

	int	(*mgmt_tx)(struct wiphy *wiphy, struct net_device *dev,
			  struct ieee80211_channel *chan, bool offchan,
			  enum nl80211_channel_type channel_type,
			  bool channel_type_valid, unsigned int wait,
			  const u8 *buf, size_t len, bool no_cck,
			  bool dont_wait_for_ack, u64 *cookie);
	int	(*mgmt_tx_cancel_wait)(struct wiphy *wiphy,
				       struct net_device *dev,
				       u64 cookie);

	int	(*set_power_mgmt)(struct wiphy *wiphy, struct net_device *dev,
				  bool enabled, int timeout);

	int	(*set_cqm_rssi_config)(struct wiphy *wiphy,
				       struct net_device *dev,
				       s32 rssi_thold, u32 rssi_hyst);

	void	(*mgmt_frame_register)(struct wiphy *wiphy,
				       struct net_device *dev,
				       u16 frame_type, bool reg);

	int	(*set_antenna)(struct wiphy *wiphy, u32 tx_ant, u32 rx_ant);
	int	(*get_antenna)(struct wiphy *wiphy, u32 *tx_ant, u32 *rx_ant);

	int	(*set_ringparam)(struct wiphy *wiphy, u32 tx, u32 rx);
	void	(*get_ringparam)(struct wiphy *wiphy,
				 u32 *tx, u32 *tx_max, u32 *rx, u32 *rx_max);

	int	(*sched_scan_start)(struct wiphy *wiphy,
				struct net_device *dev,
				struct cfg80211_sched_scan_request *request);
	int	(*sched_scan_stop)(struct wiphy *wiphy, struct net_device *dev);

	int	(*set_rekey_data)(struct wiphy *wiphy, struct net_device *dev,
				  struct cfg80211_gtk_rekey_data *data);

	int	(*tdls_mgmt)(struct wiphy *wiphy, struct net_device *dev,
			     u8 *peer, u8 action_code,  u8 dialog_token,
			     u16 status_code, const u8 *buf, size_t len);
	int	(*tdls_oper)(struct wiphy *wiphy, struct net_device *dev,
			     u8 *peer, enum nl80211_tdls_operation oper);

	int	(*probe_client)(struct wiphy *wiphy, struct net_device *dev,
				const u8 *peer, u64 *cookie);

	int	(*set_noack_map)(struct wiphy *wiphy,
				  struct net_device *dev,
				  u16 noack_map);

	struct ieee80211_channel *(*get_channel)(struct wiphy *wiphy);
	int	(*update_ft_ies)(struct wiphy *wiphy, struct net_device *dev,
				 struct cfg80211_update_ft_ies_params *ftie);

	int (*set_mac_acl)(struct wiphy *wiphy, struct net_device *dev,
			   const struct cfg80211_acl_data *params);
};


enum wiphy_flags {
	WIPHY_FLAG_CUSTOM_REGULATORY		= BIT(0),
	WIPHY_FLAG_STRICT_REGULATORY		= BIT(1),
	WIPHY_FLAG_DISABLE_BEACON_HINTS		= BIT(2),
	WIPHY_FLAG_NETNS_OK			= BIT(3),
	WIPHY_FLAG_PS_ON_BY_DEFAULT		= BIT(4),
	WIPHY_FLAG_4ADDR_AP			= BIT(5),
	WIPHY_FLAG_4ADDR_STATION		= BIT(6),
	WIPHY_FLAG_CONTROL_PORT_PROTOCOL	= BIT(7),
	WIPHY_FLAG_IBSS_RSN			= BIT(8),
	WIPHY_FLAG_MESH_AUTH			= BIT(10),
	WIPHY_FLAG_SUPPORTS_SCHED_SCAN		= BIT(11),
	WIPHY_FLAG_ENFORCE_COMBINATIONS		= BIT(12),
	WIPHY_FLAG_SUPPORTS_FW_ROAM		= BIT(13),
	WIPHY_FLAG_AP_UAPSD			= BIT(14),
	WIPHY_FLAG_SUPPORTS_TDLS		= BIT(15),
	WIPHY_FLAG_TDLS_EXTERNAL_SETUP		= BIT(16),
	WIPHY_FLAG_HAVE_AP_SME			= BIT(17),
	WIPHY_FLAG_REPORTS_OBSS			= BIT(18),
	WIPHY_FLAG_AP_PROBE_RESP_OFFLOAD	= BIT(19),
	WIPHY_FLAG_OFFCHAN_TX			= BIT(20),
	WIPHY_FLAG_HAS_REMAIN_ON_CHANNEL	= BIT(21),
};

struct ieee80211_iface_limit {
	u16 max;
	u16 types;
};

struct ieee80211_iface_combination {
	const struct ieee80211_iface_limit *limits;
	u32 num_different_channels;
	u16 max_interfaces;
	u8 n_limits;
	bool beacon_int_infra_match;
};

struct ieee80211_txrx_stypes {
	u16 tx, rx;
};

enum wiphy_wowlan_support_flags {
	WIPHY_WOWLAN_ANY		= BIT(0),
	WIPHY_WOWLAN_MAGIC_PKT		= BIT(1),
	WIPHY_WOWLAN_DISCONNECT		= BIT(2),
	WIPHY_WOWLAN_SUPPORTS_GTK_REKEY	= BIT(3),
	WIPHY_WOWLAN_GTK_REKEY_FAILURE	= BIT(4),
	WIPHY_WOWLAN_EAP_IDENTITY_REQ	= BIT(5),
	WIPHY_WOWLAN_4WAY_HANDSHAKE	= BIT(6),
	WIPHY_WOWLAN_RFKILL_RELEASE	= BIT(7),
};

struct wiphy_wowlan_support {
	u32 flags;
	int n_patterns;
	int pattern_max_len;
	int pattern_min_len;
};

/**
 * struct wiphy - wireless hardware description
 * @reg_notifier: the driver's regulatory notification callback,
 *	note that if your driver uses wiphy_apply_custom_regulatory()
 *	the reg_notifier's request can be passed as NULL
 * @regd: the driver's regulatory domain, if one was requested via
 * 	the regulatory_hint() API. This can be used by the driver
 *	on the reg_notifier() if it chooses to ignore future
 *	regulatory domain changes caused by other drivers.
 * @signal_type: signal type reported in &struct cfg80211_bss.
 * @cipher_suites: supported cipher suites
 * @n_cipher_suites: number of supported cipher suites
 * @retry_short: Retry limit for short frames (dot11ShortRetryLimit)
 * @retry_long: Retry limit for long frames (dot11LongRetryLimit)
 * @frag_threshold: Fragmentation threshold (dot11FragmentationThreshold);
 *	-1 = fragmentation disabled, only odd values >= 256 used
 * @rts_threshold: RTS threshold (dot11RTSThreshold); -1 = RTS/CTS disabled
 * @_net: the network namespace this wiphy currently lives in
 * @perm_addr: permanent MAC address of this device
 * @addr_mask: If the device supports multiple MAC addresses by masking,
 *	set this to a mask with variable bits set to 1, e.g. if the last
 *	four bits are variable then set it to 00:...:00:0f. The actual
 *	variable bits shall be determined by the interfaces added, with
 *	interfaces not matching the mask being rejected to be brought up.
 * @n_addresses: number of addresses in @addresses.
 * @addresses: If the device has more than one address, set this pointer
 *	to a list of addresses (6 bytes each). The first one will be used
 *	by default for perm_addr. In this case, the mask should be set to
 *	all-zeroes. In this case it is assumed that the device can handle
 *	the same number of arbitrary MAC addresses.
 * @registered: protects ->resume and ->suspend sysfs callbacks against
 *	unregister hardware
 * @debugfsdir: debugfs directory used for this wiphy, will be renamed
 *	automatically on wiphy renames
 * @dev: (virtual) struct device for this wiphy
 * @registered: helps synchronize suspend/resume with wiphy unregister
 * @wext: wireless extension handlers
 * @priv: driver private data (sized according to wiphy_new() parameter)
 * @interface_modes: bitmask of interfaces types valid for this wiphy,
 *	must be set by driver
 * @iface_combinations: Valid interface combinations array, should not
 *	list single interface types.
 * @n_iface_combinations: number of entries in @iface_combinations array.
 * @software_iftypes: bitmask of software interface types, these are not
 *	subject to any restrictions since they are purely managed in SW.
 * @flags: wiphy flags, see &enum wiphy_flags
 * @features: features advertised to nl80211, see &enum nl80211_feature_flags.
 * @bss_priv_size: each BSS struct has private data allocated with it,
 *	this variable determines its size
 * @max_scan_ssids: maximum number of SSIDs the device can scan for in
 *	any given scan
 * @max_sched_scan_ssids: maximum number of SSIDs the device can scan
 *	for in any given scheduled scan
 * @max_match_sets: maximum number of match sets the device can handle
 *	when performing a scheduled scan, 0 if filtering is not
 *	supported.
 * @max_scan_ie_len: maximum length of user-controlled IEs device can
 *	add to probe request frames transmitted during a scan, must not
 *	include fixed IEs like supported rates
 * @max_sched_scan_ie_len: same as max_scan_ie_len, but for scheduled
 *	scans
 * @coverage_class: current coverage class
 * @fw_version: firmware version for ethtool reporting
 * @hw_version: hardware version for ethtool reporting
 * @max_num_pmkids: maximum number of PMKIDs supported by device
 * @privid: a pointer that drivers can use to identify if an arbitrary
 *	wiphy is theirs, e.g. in global notifiers
 * @bands: information about bands/channels supported by this device
 *
 * @mgmt_stypes: bitmasks of frame subtypes that can be subscribed to or
 *	transmitted through nl80211, points to an array indexed by interface
 *	type
 *
 * @available_antennas_tx: bitmap of antennas which are available to be
 *	configured as TX antennas. Antenna configuration commands will be
 *	rejected unless this or @available_antennas_rx is set.
 *
 * @available_antennas_rx: bitmap of antennas which are available to be
 *	configured as RX antennas. Antenna configuration commands will be
 *	rejected unless this or @available_antennas_tx is set.
 *
 * @probe_resp_offload:
 *	 Bitmap of supported protocols for probe response offloading.
 *	 See &enum nl80211_probe_resp_offload_support_attr. Only valid
 *	 when the wiphy flag @WIPHY_FLAG_AP_PROBE_RESP_OFFLOAD is set.
 *
 * @max_remain_on_channel_duration: Maximum time a remain-on-channel operation
 *	may request, if implemented.
 *
 * @wowlan: WoWLAN support information
 *
 * @ap_sme_capa: AP SME capabilities, flags from &enum nl80211_ap_sme_features.
 * @ht_capa_mod_mask:  Specify what ht_cap values can be over-ridden.
 *	If null, then none can be over-ridden.
 *
 * @max_acl_mac_addrs: Maximum number of MAC addresses that the device
 *	supports for ACL.
 */
struct wiphy {
	

	
	u8 perm_addr[ETH_ALEN];
	u8 addr_mask[ETH_ALEN];

	struct mac_address *addresses;

	const struct ieee80211_txrx_stypes *mgmt_stypes;

	const struct ieee80211_iface_combination *iface_combinations;
	int n_iface_combinations;
	u16 software_iftypes;

	u16 n_addresses;

	
	u16 interface_modes;

	u16 max_acl_mac_addrs;

	u32 flags, features;

	u32 ap_sme_capa;

	enum cfg80211_signal_type signal_type;

	int bss_priv_size;
	u8 max_scan_ssids;
	u8 max_sched_scan_ssids;
	u8 max_match_sets;
	u16 max_scan_ie_len;
	u16 max_sched_scan_ie_len;

	int n_cipher_suites;
	const u32 *cipher_suites;

	u8 retry_short;
	u8 retry_long;
	u32 frag_threshold;
	u32 rts_threshold;
	u8 coverage_class;

	char fw_version[ETHTOOL_BUSINFO_LEN];
	u32 hw_version;

	struct wiphy_wowlan_support wowlan;

	u16 max_remain_on_channel_duration;

	u8 max_num_pmkids;

	u32 available_antennas_tx;
	u32 available_antennas_rx;

	u32 probe_resp_offload;

	const void *privid;

	struct ieee80211_supported_band *bands[IEEE80211_NUM_BANDS];

	
	int (*reg_notifier)(struct wiphy *wiphy,
			    struct regulatory_request *request);

	

	const struct ieee80211_regdomain *regd;

	struct device dev;

	
	bool registered;

	
	struct dentry *debugfsdir;

	const struct ieee80211_ht_cap *ht_capa_mod_mask;

#ifdef CONFIG_NET_NS
	
	struct net *_net;
#endif

#ifdef CONFIG_CFG80211_WEXT
	const struct iw_handler_def *wext;
#endif

	char priv[0] __attribute__((__aligned__(NETDEV_ALIGN)));
};

static inline struct net *wiphy_net(struct wiphy *wiphy)
{
	return read_pnet(&wiphy->_net);
}

static inline void wiphy_net_set(struct wiphy *wiphy, struct net *net)
{
	write_pnet(&wiphy->_net, net);
}

static inline void *wiphy_priv(struct wiphy *wiphy)
{
	BUG_ON(!wiphy);
	return &wiphy->priv;
}

static inline struct wiphy *priv_to_wiphy(void *priv)
{
	BUG_ON(!priv);
	return container_of(priv, struct wiphy, priv);
}

static inline void set_wiphy_dev(struct wiphy *wiphy, struct device *dev)
{
	wiphy->dev.parent = dev;
}

static inline struct device *wiphy_dev(struct wiphy *wiphy)
{
	return wiphy->dev.parent;
}

static inline const char *wiphy_name(const struct wiphy *wiphy)
{
	return dev_name(&wiphy->dev);
}

struct wiphy *wiphy_new(const struct cfg80211_ops *ops, int sizeof_priv);

extern int wiphy_register(struct wiphy *wiphy);

extern void wiphy_unregister(struct wiphy *wiphy);

extern void wiphy_free(struct wiphy *wiphy);

struct cfg80211_conn;
struct cfg80211_internal_bss;
struct cfg80211_cached_keys;

struct wireless_dev {
	struct wiphy *wiphy;
	enum nl80211_iftype iftype;

	
	struct list_head list;
	struct net_device *netdev;

	struct list_head mgmt_registrations;
	spinlock_t mgmt_registrations_lock;

	struct mutex mtx;

	struct work_struct cleanup_work;

	bool use_4addr;

	
	u8 ssid[IEEE80211_MAX_SSID_LEN];
	u8 ssid_len, mesh_id_len, mesh_id_up_len;
	enum {
		CFG80211_SME_IDLE,
		CFG80211_SME_CONNECTING,
		CFG80211_SME_CONNECTED,
	} sme_state;
	struct cfg80211_conn *conn;
	struct cfg80211_cached_keys *connect_keys;

	struct list_head event_list;
	spinlock_t event_lock;

	struct cfg80211_internal_bss *current_bss; 
	struct ieee80211_channel *channel;

	bool ps;
	int ps_timeout;

	int beacon_interval;

	u32 ap_unexpected_nlpid;

#ifdef CONFIG_CFG80211_WEXT
	
	struct {
		struct cfg80211_ibss_params ibss;
		struct cfg80211_connect_params connect;
		struct cfg80211_cached_keys *keys;
		u8 *ie;
		size_t ie_len;
		u8 bssid[ETH_ALEN], prev_bssid[ETH_ALEN];
		u8 ssid[IEEE80211_MAX_SSID_LEN];
		s8 default_key, default_mgmt_key;
		bool prev_bssid_valid;
	} wext;
#endif
};

static inline void *wdev_priv(struct wireless_dev *wdev)
{
	BUG_ON(!wdev);
	return wiphy_priv(wdev->wiphy);
}


extern int ieee80211_channel_to_frequency(int chan, enum ieee80211_band band);

extern int ieee80211_frequency_to_channel(int freq);

extern struct ieee80211_channel *__ieee80211_get_channel(struct wiphy *wiphy,
							 int freq);
static inline struct ieee80211_channel *
ieee80211_get_channel(struct wiphy *wiphy, int freq)
{
	return __ieee80211_get_channel(wiphy, freq);
}

struct ieee80211_rate *
ieee80211_get_response_rate(struct ieee80211_supported_band *sband,
			    u32 basic_rates, int bitrate);


struct radiotap_align_size {
	uint8_t align:4, size:4;
};

struct ieee80211_radiotap_namespace {
	const struct radiotap_align_size *align_size;
	int n_bits;
	uint32_t oui;
	uint8_t subns;
};

struct ieee80211_radiotap_vendor_namespaces {
	const struct ieee80211_radiotap_namespace *ns;
	int n_ns;
};


struct ieee80211_radiotap_iterator {
	struct ieee80211_radiotap_header *_rtheader;
	const struct ieee80211_radiotap_vendor_namespaces *_vns;
	const struct ieee80211_radiotap_namespace *current_namespace;

	unsigned char *_arg, *_next_ns_data;
	__le32 *_next_bitmap;

	unsigned char *this_arg;
	int this_arg_index;
	int this_arg_size;

	int is_radiotap_ns;

	int _max_length;
	int _arg_index;
	uint32_t _bitmap_shifter;
	int _reset_on_ext;
};

extern int ieee80211_radiotap_iterator_init(
	struct ieee80211_radiotap_iterator *iterator,
	struct ieee80211_radiotap_header *radiotap_header,
	int max_length, const struct ieee80211_radiotap_vendor_namespaces *vns);

extern int ieee80211_radiotap_iterator_next(
	struct ieee80211_radiotap_iterator *iterator);


extern const unsigned char rfc1042_header[6];
extern const unsigned char bridge_tunnel_header[6];

unsigned int ieee80211_get_hdrlen_from_skb(const struct sk_buff *skb);

unsigned int __attribute_const__ ieee80211_hdrlen(__le16 fc);


int ieee80211_data_to_8023(struct sk_buff *skb, const u8 *addr,
			   enum nl80211_iftype iftype);

int ieee80211_data_from_8023(struct sk_buff *skb, const u8 *addr,
			     enum nl80211_iftype iftype, u8 *bssid, bool qos);

void ieee80211_amsdu_to_8023s(struct sk_buff *skb, struct sk_buff_head *list,
			      const u8 *addr, enum nl80211_iftype iftype,
			      const unsigned int extra_headroom,
			      bool has_80211_header);

unsigned int cfg80211_classify8021d(struct sk_buff *skb);

const u8 *cfg80211_find_ie(u8 eid, const u8 *ies, int len);

const u8 *cfg80211_find_vendor_ie(unsigned int oui, u8 oui_type,
				  const u8 *ies, int len);


extern int regulatory_hint(struct wiphy *wiphy, const char *alpha2);

extern void wiphy_apply_custom_regulatory(
	struct wiphy *wiphy,
	const struct ieee80211_regdomain *regd);

extern int freq_reg_info(struct wiphy *wiphy,
			 u32 center_freq,
			 u32 desired_bw_khz,
			 const struct ieee80211_reg_rule **reg_rule);


void cfg80211_scan_done(struct cfg80211_scan_request *request, bool aborted);

void cfg80211_sched_scan_results(struct wiphy *wiphy);

void cfg80211_sched_scan_stopped(struct wiphy *wiphy);

struct cfg80211_bss * __must_check
cfg80211_inform_bss_frame(struct wiphy *wiphy,
			  struct ieee80211_channel *channel,
			  struct ieee80211_mgmt *mgmt, size_t len,
			  s32 signal, gfp_t gfp);

struct cfg80211_bss * __must_check
cfg80211_inform_bss(struct wiphy *wiphy,
		    struct ieee80211_channel *channel,
		    const u8 *bssid, u64 tsf, u16 capability,
		    u16 beacon_interval, const u8 *ie, size_t ielen,
		    s32 signal, gfp_t gfp);

struct cfg80211_bss *cfg80211_get_bss(struct wiphy *wiphy,
				      struct ieee80211_channel *channel,
				      const u8 *bssid,
				      const u8 *ssid, size_t ssid_len,
				      u16 capa_mask, u16 capa_val);
static inline struct cfg80211_bss *
cfg80211_get_ibss(struct wiphy *wiphy,
		  struct ieee80211_channel *channel,
		  const u8 *ssid, size_t ssid_len)
{
	return cfg80211_get_bss(wiphy, channel, NULL, ssid, ssid_len,
				WLAN_CAPABILITY_IBSS, WLAN_CAPABILITY_IBSS);
}

struct cfg80211_bss *cfg80211_get_mesh(struct wiphy *wiphy,
				       struct ieee80211_channel *channel,
				       const u8 *meshid, size_t meshidlen,
				       const u8 *meshcfg);
void cfg80211_ref_bss(struct cfg80211_bss *bss);

void cfg80211_put_bss(struct cfg80211_bss *bss);

void cfg80211_unlink_bss(struct wiphy *wiphy, struct cfg80211_bss *bss);

void cfg80211_send_rx_auth(struct net_device *dev, const u8 *buf, size_t len);

void cfg80211_send_auth_timeout(struct net_device *dev, const u8 *addr);

void cfg80211_send_rx_assoc(struct net_device *dev, struct cfg80211_bss *bss,
			    const u8 *buf, size_t len);

void cfg80211_send_assoc_timeout(struct net_device *dev, const u8 *addr);

void cfg80211_send_deauth(struct net_device *dev, const u8 *buf, size_t len);

void __cfg80211_send_deauth(struct net_device *dev, const u8 *buf, size_t len);

void cfg80211_send_disassoc(struct net_device *dev, const u8 *buf, size_t len);

void __cfg80211_send_disassoc(struct net_device *dev, const u8 *buf,
	size_t len);

void cfg80211_send_unprot_deauth(struct net_device *dev, const u8 *buf,
				 size_t len);

void cfg80211_send_unprot_disassoc(struct net_device *dev, const u8 *buf,
				   size_t len);

void cfg80211_michael_mic_failure(struct net_device *dev, const u8 *addr,
				  enum nl80211_key_type key_type, int key_id,
				  const u8 *tsc, gfp_t gfp);

void cfg80211_ibss_joined(struct net_device *dev, const u8 *bssid, gfp_t gfp);

void cfg80211_notify_new_peer_candidate(struct net_device *dev,
		const u8 *macaddr, const u8 *ie, u8 ie_len, gfp_t gfp);


void wiphy_rfkill_set_hw_state(struct wiphy *wiphy, bool blocked);

void wiphy_rfkill_start_polling(struct wiphy *wiphy);

void wiphy_rfkill_stop_polling(struct wiphy *wiphy);

#ifdef CONFIG_NL80211_TESTMODE

struct sk_buff *cfg80211_testmode_alloc_reply_skb(struct wiphy *wiphy,
						  int approxlen);

int cfg80211_testmode_reply(struct sk_buff *skb);

struct sk_buff *cfg80211_testmode_alloc_event_skb(struct wiphy *wiphy,
						  int approxlen, gfp_t gfp);

void cfg80211_testmode_event(struct sk_buff *skb, gfp_t gfp);

#define CFG80211_TESTMODE_CMD(cmd)	.testmode_cmd = (cmd),
#define CFG80211_TESTMODE_DUMP(cmd)	.testmode_dump = (cmd),
#else
#define CFG80211_TESTMODE_CMD(cmd)
#define CFG80211_TESTMODE_DUMP(cmd)
#endif

void cfg80211_connect_result(struct net_device *dev, const u8 *bssid,
			     const u8 *req_ie, size_t req_ie_len,
			     const u8 *resp_ie, size_t resp_ie_len,
			     u16 status, gfp_t gfp);

void cfg80211_roamed(struct net_device *dev,
		     struct ieee80211_channel *channel,
		     const u8 *bssid,
		     const u8 *req_ie, size_t req_ie_len,
		     const u8 *resp_ie, size_t resp_ie_len, gfp_t gfp);

void cfg80211_roamed_bss(struct net_device *dev, struct cfg80211_bss *bss,
			 const u8 *req_ie, size_t req_ie_len,
			 const u8 *resp_ie, size_t resp_ie_len, gfp_t gfp);

void cfg80211_disconnected(struct net_device *dev, u16 reason,
			   u8 *ie, size_t ie_len, gfp_t gfp);

void cfg80211_ready_on_channel(struct net_device *dev, u64 cookie,
			       struct ieee80211_channel *chan,
			       enum nl80211_channel_type channel_type,
			       unsigned int duration, gfp_t gfp);

void cfg80211_remain_on_channel_expired(struct net_device *dev,
					u64 cookie,
					struct ieee80211_channel *chan,
					enum nl80211_channel_type channel_type,
					gfp_t gfp);


void cfg80211_new_sta(struct net_device *dev, const u8 *mac_addr,
		      struct station_info *sinfo, gfp_t gfp);

void cfg80211_del_sta(struct net_device *dev, const u8 *mac_addr, gfp_t gfp);

bool cfg80211_rx_mgmt(struct net_device *dev, int freq, int sig_dbm,
		      const u8 *buf, size_t len, gfp_t gfp);

void cfg80211_mgmt_tx_status(struct net_device *dev, u64 cookie,
			     const u8 *buf, size_t len, bool ack, gfp_t gfp);


void cfg80211_cqm_rssi_notify(struct net_device *dev,
			      enum nl80211_cqm_rssi_threshold_event rssi_event,
			      gfp_t gfp);

void cfg80211_cqm_pktloss_notify(struct net_device *dev,
				 const u8 *peer, u32 num_packets, gfp_t gfp);

void cfg80211_gtk_rekey_notify(struct net_device *dev, const u8 *bssid,
			       const u8 *replay_ctr, gfp_t gfp);

void cfg80211_pmksa_candidate_notify(struct net_device *dev, int index,
				     const u8 *bssid, bool preauth, gfp_t gfp);

bool cfg80211_rx_spurious_frame(struct net_device *dev,
				const u8 *addr, gfp_t gfp);

bool cfg80211_rx_unexpected_4addr_frame(struct net_device *dev,
					const u8 *addr, gfp_t gfp);

void cfg80211_probe_status(struct net_device *dev, const u8 *addr,
			   u64 cookie, bool acked, gfp_t gfp);

void cfg80211_report_obss_beacon(struct wiphy *wiphy,
				 const u8 *frame, size_t len,
				 int freq, int sig_dbm, gfp_t gfp);

int cfg80211_can_beacon_sec_chan(struct wiphy *wiphy,
				 struct ieee80211_channel *chan,
				 enum nl80211_channel_type channel_type);

#ifdef CONFIG_QUALCOMM_WLAN
u32 cfg80211_calculate_bitrate(struct rate_info *rate);
#else
u16 cfg80211_calculate_bitrate(struct rate_info *rate);
#endif



#define wiphy_printk(level, wiphy, format, args...)		\
	dev_printk(level, &(wiphy)->dev, format, ##args)
#define wiphy_emerg(wiphy, format, args...)			\
	dev_emerg(&(wiphy)->dev, format, ##args)
#define wiphy_alert(wiphy, format, args...)			\
	dev_alert(&(wiphy)->dev, format, ##args)
#define wiphy_crit(wiphy, format, args...)			\
	dev_crit(&(wiphy)->dev, format, ##args)
#define wiphy_err(wiphy, format, args...)			\
	dev_err(&(wiphy)->dev, format, ##args)
#define wiphy_warn(wiphy, format, args...)			\
	dev_warn(&(wiphy)->dev, format, ##args)
#define wiphy_notice(wiphy, format, args...)			\
	dev_notice(&(wiphy)->dev, format, ##args)
#define wiphy_info(wiphy, format, args...)			\
	dev_info(&(wiphy)->dev, format, ##args)

#define wiphy_debug(wiphy, format, args...)			\
	wiphy_printk(KERN_DEBUG, wiphy, format, ##args)

#define wiphy_dbg(wiphy, format, args...)			\
	dev_dbg(&(wiphy)->dev, format, ##args)

#if defined(VERBOSE_DEBUG)
#define wiphy_vdbg	wiphy_dbg
#else
#define wiphy_vdbg(wiphy, format, args...)				\
({									\
	if (0)								\
		wiphy_printk(KERN_DEBUG, wiphy, format, ##args);	\
	0;								\
})
#endif

#define wiphy_WARN(wiphy, format, args...)			\
	WARN(1, "wiphy: %s\n" format, wiphy_name(wiphy), ##args);

#endif 
