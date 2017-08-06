# Arduino EtherShield Relay with REST Web interface #

This arduino webserver works with the follwowing 'official' clients:
* react-arduino_relay
    * [github](https://github.com/3Tron/react-arduino_relay)
    * [npm](https://www.npmjs.com/package/relay.ruijs.fr)
* node_relay_client
    * [npm](https://www.npmjs.com/package/node_relay_client)

## Example ##
Internet routers hangs sometimes for various reasons:
* Your provider give you crappy hardware.
* You IP range is currently being attacked or flooded for various reasons.

Mostly the router comes back to normal operation by itself but not always...too often it doesn't come back and even more annoying when not being at home to reset it manually. 

This is actually the mean reason why I created this project.

In combination with 'checkip' [npm](https://www.npmjs.com/package/node_checkip) you can let your arduino shut down your router en power it back on.