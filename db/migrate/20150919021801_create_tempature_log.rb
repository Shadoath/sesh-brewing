class CreateTempatureLog < ActiveRecord::Migration
  def change
    create_table :tempature_logs do |t|
      t.string :tempature_c
      t.timestamps null: false
    end
  end
end
