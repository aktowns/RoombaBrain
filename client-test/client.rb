#!/usr/bin/env ruby 

require 'coap'
require 'json'

class Roomba 
  def initialize(address)
    @address = address
    @client = CoAP::Client.new(host: '192.168.1.110')
  end

  def start 
    p @client.get('/roomba/cmd')
    send_cmd('start')
  end

  private
  def send_cmd(cmd) 
    obj = {
      id: 1,
      method: cmd,
      arguments: []
    }
    @client.post('/roomba/cmd', nil, nil, obj.to_json)
  end
end


roomba = Roomba.new('192.168.1.110')
p roomba.start.payload
