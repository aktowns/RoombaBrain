#!/usr/bin/env ruby 

require 'coap'
require 'json'

obj = {
  id: 1,
  method: "test/cmd",
  arguments: []
}

c = CoAP::Client.new(host: '192.168.1.110')
p c.get('/roomba/cmd')
p c.post('/roomba/cmd', nil, nil, obj.to_json)

