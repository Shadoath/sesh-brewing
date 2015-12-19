# This file should contain all the record creation needed to seed the database with its default values.
# The data can then be loaded with the rake db:seed (or created alongside the db with db:setup).
#
# Examples:
#
#   cities = City.create([{ name: 'Chicago' }, { name: 'Copenhagen' }])
#   Mayor.create(name: 'Emanuel', city: cities.first)

  Arduino.find_or_create_by(name: "Test One", sensor_type: 0, current_tempature: 42, target_tempature: 69, is_running?: true, relay_on?: false, control_direction_heating?: true, address: 1)
  Arduino.find_or_create_by(name: "Test Two", sensor_type: 1, current_tempature: nil, target_tempature: 99, is_running?: false, relay_on?: false, control_direction_heating?: true, address: 2)