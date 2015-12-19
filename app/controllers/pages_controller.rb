class PagesController < ApplicationController
  # For APIs, you may want to use :null_session instead.
  def home
    @arduinos = Arduino.all
  end

end
