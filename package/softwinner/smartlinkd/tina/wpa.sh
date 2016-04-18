#/bin/sh
#$1 ssid 
#$2 psk 
#$3 key_mgmt, eg: WPA-PSK, WPA_EAP, None 
#$4 ifName, eg:wlan0, wlan1...  default:wlan0

export ifName=wlan0
export configPath=/etc/wifi
export configFile=$configPath/wpa_supplicant.conf
export wpaInterface=$configPath/sockets

check_config_file(){
	[ -f $configFile ] || {
		echo "ctrl_interface=$wpaInterface" > $configFile
		echo "update_config=1" >> $configFile
	}
}

check_wlan(){ # return:  up->0, down->1
	ifconfig | grep $1
	return $?
}

check_wpa_supplicant(){ # return: wpa_supplicant aleady run ->0, no run ->1
	ps | grep wpa_supplicant | awk '{print $5}' | grep wpa_supplicant
	return $?
}

do_wpa_cli(){
	wpa_cli -p$wpaInterface -i$ifName $@
	return $?
}

check_ssid_in_supplicant(){ # $1:ssid, $2: current exsist networks, return net_network id
	let id=0
	while [ $id -le $2 ]
	do
		ssid=`do_wpa_cli get_network $id ssid`
		[ $1 = $ssid ] && break
		let id++
	done
	return $id
}

get_max_priority(){ # $1 current exsist networks
	let id=0
	let max_priority=0
	while [ $id -le $1 ]
	do
		priority=`do_wpa_cli get_network $id priority`
		if [ $max_priority -lt $priority ]; then
			max_priority=$priority
		fi
		let id++
	done
	echo $max_priority
	return $max_priority
}

[ $# -lt 3 ] && exit -1
[ $# -eq 4 ] && export ifName=$4
set_ssid=$1
set_psk=$2
set_key_mgmt=$3

check_config_file

check_wlan $ifName
[ $? -eq 1 ] && {
	echo "$ifName not up, now ifconfig $ifName up"
	ifconfig $ifName up
	[ $? -eq 1 ] && exit -1
}

check_wpa_supplicant
[ $? -eq 1 ] && {
	rm -rf $wpaInterface
	wpa_supplicant -i$ifName -Dnl80211 -c$configFile -B
	let timeout=0
	while [ ! -e $wpaInterface ]
	do
		sleep 1
		let timeout=timeout+1
		[ $timeout -gt 10 ] && exit -1;
	done
}

add_network_id=`do_wpa_cli add_network`
let cur_networks=$add_network_id-1

#do_wpa_cli remove_network $add_network_id

let set_priority=`get_max_priority $cur_networks`+1

echo $set_priority

check_ssid_in_supplicant \"$set_ssid\" $cur_networks
let set_network_id=$?
[ $set_network_id -eq $add_network_id ] || {
	do_wpa_cli remove_network $add_network_id
}

do_wpa_cli set_network $set_network_id ssid \"$set_ssid\"
do_wpa_cli set_network $set_network_id psk \"$set_psk\"
do_wpa_cli set_network $set_network_id key_mgmt $set_key_mgmt
do_wpa_cli set_network $set_network_id priority $set_priority

do_wpa_cli save_config

do_wpa_cli enable_network $set_network_id
#do_wpa_cli scan
