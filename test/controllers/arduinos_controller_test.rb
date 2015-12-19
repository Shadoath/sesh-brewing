require 'test_helper'

class ArduinosControllerTest < ActionController::TestCase
  setup do
    @arduino = arduinos(:one)
  end

  test "should get index" do
    get :index
    assert_response :success
    assert_not_nil assigns(:arduinos)
  end

  test "should get new" do
    get :new
    assert_response :success
  end

  test "should create arduino" do
    assert_difference('Arduino.count') do
      post :create, arduino: { address: @arduino.address, control_direction_heating?: @arduino.control_direction_heating?, current_tempature: @arduino.current_tempature, is_running?: @arduino.is_running?, name: @arduino.name, relay_on?: @arduino.relay_on?, sensor_type: @arduino.sensor_type, target_tempature: @arduino.target_tempature }
    end

    assert_redirected_to arduino_path(assigns(:arduino))
  end

  test "should show arduino" do
    get :show, id: @arduino
    assert_response :success
  end

  test "should get edit" do
    get :edit, id: @arduino
    assert_response :success
  end

  test "should update arduino" do
    patch :update, id: @arduino, arduino: { address: @arduino.address, control_direction_heating?: @arduino.control_direction_heating?, current_tempature: @arduino.current_tempature, is_running?: @arduino.is_running?, name: @arduino.name, relay_on?: @arduino.relay_on?, sensor_type: @arduino.sensor_type, target_tempature: @arduino.target_tempature }
    assert_redirected_to arduino_path(assigns(:arduino))
  end

  test "should destroy arduino" do
    assert_difference('Arduino.count', -1) do
      delete :destroy, id: @arduino
    end

    assert_redirected_to arduinos_path
  end
end
