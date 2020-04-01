require 'sinatra'
require 'sinatra/base'
require 'json'

class App < Sinatra::Base
  get '/' do
    'Is this, IoT?'
  end

  post '/post-sensor-data' do
    #puts JSON.pretty_generate(request.env)
    #puts "+++++++++++++++++++++"
    data = JSON.parse( request.body.read.to_s )
    puts data
    'thanks'
  end
end
