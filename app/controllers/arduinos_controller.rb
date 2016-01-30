class ArduinosController < ApplicationController
  before_action :set_arduino, only: [:show, :edit, :update, :destroy]

  def get_tempature

  end
  # GET /arduinos
  def index
    @arduinos = Arduino.all
  end

  # GET /arduinos/1
  def show
  end

  # GET /arduinos/new
  def new
    @arduino = Arduino.new
  end

  # GET /arduinos/1/edit
  def edit
  end

  # POST /arduinos
  def create
    @arduino = Arduino.new(arduino_params)

    if @arduino.save
      redirect_to @arduino, notice: 'Arduino was successfully created.'
    else
      render :new
    end
  end

  # PATCH/PUT /arduinos/1
  def update
    if @arduino.update(arduino_params)
      redirect_to @arduino, notice: 'Arduino was successfully updated.'
    else
      render :edit
    end
  end

  # DELETE /arduinos/1
  def destroy
    @arduino.destroy
    redirect_to arduinos_url, notice: 'Arduino was successfully destroyed.'
  end

  private
    # Use callbacks to share common setup or constraints between actions.
    def set_arduino
      @arduino = Arduino.find(params[:id])
    end

    # Only allow a trusted parameter "white list" through.
    def arduino_params
      params.require(:arduino).permit(:name, :sensor_type, :current_tempature, :target_tempature, :is_running?, :relay_on?, :control_direction_heating?, :address)
    end
end
