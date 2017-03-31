function pretty_bytes ( data ) {

	var input = parseInt(data, 10);

	var str = input + " B";
	var comp = 1024;
	if ( input > comp ) {
		input = parseInt(input/comp, 10);
		str = input + " KB";
			console.log("str: " + str);
		if ( input > comp ){
			input = parseInt(input/comp, 10);
			str = input + " MB";
			if ( input > comp ){
				input = parseInt(input/comp, 10);
				str = input + " GB";
				if ( input > comp ){
					input = parseInt(input/comp, 10);
					str = input + " TB";
				}
			}
		}
	}

	return str;


}