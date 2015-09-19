class CreateTempatureLog < ActiveRecord::Migration
  def change
    create_table :tempature_logs do |t|
      t.temp :string
    end
  end
end
