class PagesController < ApplicationController
  # For APIs, you may want to use :null_session instead.
  def home
    @lastTemp = TempatureLog.last
  end

end
