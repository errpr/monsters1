require 'ERB'

template_files = Dir.entries(".").select { |f| f.match? /.+erb/ }

template_files.each do |file|
  file_text = File.read(file)
  template = ERB.new(file_text, 3, '>')
  output_name = file[0..-5]
  output = File.open(output_name, "w")
  output.write(template.result)
end
