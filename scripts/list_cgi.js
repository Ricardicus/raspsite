function sort_items_based_on_path_func(lhs, rhs){
	return lhs["path"].localeCompare(rhs["path"]);
}

function list(path) {

	http_get("/list.cgi?action=list&path=" + path, "text", 
			function(response){
			var html = "";
			var rows = response.split('\n');

			var doubleDotFound = false;
			var doubleDotStat = {};
			var items = [];

			for ( var i = 1; i < rows.length ; i++ ){

				var data = rows[i].split(/[ ,]+/);
				var doubleDot = false;
				var stats = {};
				
				stats["perms"] = data[0];
				stats["size"] = data[4];
				stats["disp"] = data[data.length - 1];

				var url = data[data.length - 1];

				if ( url == "." ) {

					url = path;

				} else if ( url == ".." ){

					var doms = path.split("/");

					var url = "/";
					for ( var n = 1; n < doms.length - 2; n++) {
						url += doms[n];
						if ( n != doms.length - 3) 
							url += "/";
					}
					
					if ( url === 'undefined' ){
						url = "/";
					}

					doubleDotFound = true;
					doubleDot = true;

					doubleDotStat["perms"] = data[0];
					doubleDotStat["size"] = data[4];
					doubleDotStat["disp"] = data[data.length - 1];
					doubleDotStat["path"] = url;

				}

				stats["path"] = url;

				if ( !doubleDot) {
					items.push(stats);
				}

			}

			items = items.sort(sort_items_based_on_path_func);

			if (doubleDotFound) {
				html += '<tr><td>'+doubleDotStat["perms"] +'</td><td>' + pretty_bytes(doubleDotStat["size"]) + '</td><td>' + 
				doubleDotStat["disp"] + '</td><td><img class="overout" type="dir" path="' +  ( doubleDotStat["disp"] == ".." ? doubleDotStat["path"] : path + stats["path"] + "/" ) + 
				'" src="' + (doubleDotStat["disp"] == ".." ? '/style/Arrow_Back.png' : '/style/Arrow_Forward.png') + '"></td></tr>';
			}

			for ( var i = 0; i < items.length; i++) {

				var stats = items[i];

				if ( stats["perms"].charAt(0) == 'd' ){
					html += '<tr><td>'+stats["perms"] +'</td><td>' + pretty_bytes(stats["size"]) + '</td><td>' + 
					stats["disp"] + '</td><td><img class="overout" type="dir" path="' +  ( stats["disp"] == ".." ? stats["path"] : path + stats["path"] + "/" ) + 
					'" src="' + (stats["disp"] == ".." ? '/style/Arrow_Back.png' : '/style/Arrow_Forward.png') + '"></td></tr>';
				} else {
					html += '<tr><td>'+stats["perms"] +'</td><td>' + pretty_bytes( stats["size"] ) + '</td><td>' + 
					stats["disp"] + '</td><td><img class="overout" type="file" path="' + path + stats["path"] + 
					'" src="/style/download.png"></td></tr>';
				}
			}

			$("#files-table-tbody").html(html);

			handlers();

		}, 
		function(){
			alert("failed!")
		}
	);


}