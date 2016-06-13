var lineReader = require('readline').createInterface({
  input: require('fs').createReadStream('cmc.data'),
  output: require('fs').createWriteStream('cmc-proc.data')
});

lineReader.on('line', function (line) {
	cmc(line);
});

lineReader.on('close', function(){
	lineReader.output.end();
})

function abalone(line){
	var splitted = line.split(',');
  	var rings = splitted[splitted.length - 1];
  	rings = rings < 9 ? 0 : (rings < 15 ? 1 : 2); 
  	splitted.unshift(rings);
  	splitted.splice(splitted.length - 1, 1);
  	lineReader.output.write(splitted.join()+"\r\n");
}

function cmc(line){
	var splitted = line.split(',');
  	var method = splitted[splitted.length - 1] - 1;
  	splitted.unshift(method);
  	splitted.splice(splitted.length - 1, 1);
  	lineReader.output.write(splitted.join()+"\r\n");
}

function adult(line){
  var splitted = line.split(',');
  var trimmed = [];
  splitted.forEach(function(attr){
    trimmed.push(attr.trim());
  })
    var income = trimmed[trimmed.length - 1];
    income = income == "<=50K" ? 0 : 1;
    trimmed.unshift(income);
    trimmed.splice(trimmed.length - 1, 1);
    lineReader.output.write(trimmed.join()+"\r\n");
}
