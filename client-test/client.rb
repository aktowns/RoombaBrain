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
out = c.post('/roomba/cmd', nil, nil, obj.to_json)
p out 
puts out.payload
