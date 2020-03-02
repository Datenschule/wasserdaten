require 'sinatra'
require 'sinatra/base'
require 'json'

class App < Sinatra::Base
  get '/' do
    'Is this, IoT?'
  end

  post '/send-data' do
    data = JSON.parse( request.body.read.to_s )
    puts headers
    puts data
    'thanks'
  end
end
