var Events = function(id, size) {
    this.id = id
    this.size = size;
    this.element = $('#l-eventos')
    
    this.traducciones = {};
    this.traducciones['A'] = "Agua";
    this.traducciones['I'] = "Apuntado";
    this.traducciones['T'] = "Tocado";
    this.traducciones['H'] = "Hundido";

    /* Para mantener el estado de mi tablero */
    this.tiles = new Array(size);		
	for(x=0;x<size; x++) {
        this.tiles[x] = new Array(size);
        for(y=0;y<size; y++) {
            this.tiles[x][y] = Codigo.Agua;
        }
    }
}

Events.prototype.set_player_board = function(boats) {
	for (b = 0; b < boats.length; b++) {
		for (i = 0; i < boats[b].length; i++) {
			x = boats[b][i]['x'];
			y = boats[b][i]['y'];
			this.tiles[x][y] = Codigo.Bote;
		}
	}
}

Events.prototype.draw_player_board = function() {
	str2 = "<table id='mboard'>"
	for(x=0;x<this.size;x++){
		str2 += '<tr id=mtr>';
		for(y=0;y<this.size;y++){
			str2 += '<td id=mtd class="'+this.tiles[x][y]+'"></td>';
		}
		str2 += '</tr>';
	}
	str2 += '</table>'
	paneles.myboats.html(str2);
}


Events.prototype.new_event = function(event_info) {
    target=event_info['t_id']
    source=event_info['s_id']
    status=event_info['status'];
    traduccion = this.traducciones[status]
    x = event_info['coords']['x'];
    y = event_info['coords']['y'];
    str ='';
    inc = true;
    if (this.id == target) {
    	inc = true;
    	str = 'Incoming: ' + traduccion + ' en x: ' + x + ' y: ' + y + ' (' + game.players[source].name + ')';
    } else {
    	inc = false;
    	str = 'Outgoing: ' + traduccion + ' a x: ' + x + ' y: ' + y + ' (' + game.players[target].name + ')';
    }
    color = 'black;';
    if (status == 'I' && inc) {
    	color = 'yellow;';
    	this.tiles[x][y] = Codigo.Incoming;
    } else if ((status == 'T' || status == 'H') && inc) {
    	color = 'red;';
    	if (status == 'T') {
    		this.tiles[x][y] = Codigo.Tocado;
    	} else {
    		this.tiles[x][y] = Codigo.Hundido;
    	}
    } else if ((status == 'T' || status == 'H' && !inc)) {
    	color = 'rgb(0, 255, 0);';
    } else if (status == 'A' && inc) {
    	this.tiles[x][y] = Codigo.Descubierto;
    }
    
    this.element.prepend('<li style="color:' + color +'">'+str+'</li>');
    this.draw_player_board();

}
