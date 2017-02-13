function http_get(url, dataType, successCallback, errorCallback){

	$.ajax({
		type: "GET",
		url: url,
		dataType: dataType,
		success: function(response){
			if ( !(typeof successCallback === 'undefined') ) {
				successCallback(response);
			}
		},
		error: function(){
			if ( !(typeof errorCallback === 'undefined') ) {
				errorCallback();
			}
		}
	});

}

function http_post(url, dataType, data, successCallback, errorCallback){

	$.ajax({
		type: "POST",
		url: url,
		dataType: dataType,
		data: data,
		success: function(response){
			if ( !(typeof successCallback === 'undefined') ) {
				successCallback(response);
			}
		},
		error: function(){
			if ( !(typeof errorCallback === 'undefined') ) {
				errorCallback();
			}
		}
	});

}