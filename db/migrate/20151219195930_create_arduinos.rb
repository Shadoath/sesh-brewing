class CreateArduinos < ActiveRecord::Migration
  def change
    create_table :arduinos do |t|
      t.string :name
      t.integer :sensor_type
      t.float :current_tempature
      t.float :target_tempature
      t.boolean :is_running?
      t.boolean :relay_on?
      t.boolean :control_direction_heating?
      t.integer :address

      t.timestamps null: false
    end
  end
end
