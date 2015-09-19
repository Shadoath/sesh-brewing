class Api::V1::ApiController < ApplicationController
  # before_action :authenticate_write_request

  def authenticate_write_request
    head :unauthorized unless Authorizer.authorized?(request.headers['Token'])
  end

end

