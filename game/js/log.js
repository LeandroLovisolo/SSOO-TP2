function mostrar_log() {
    $('#log').slideDown();
    $('#abrir_log').hide();
    $('#cerrar_log').show();
    return false;
}

function ocultar_log() {
    $('#log').slideUp();
    $('#cerrar_log').hide();
    $('#abrir_log').show();
    return false;
}

function loguear(s, color) {
    if (arguments.length != 2) {
        color = 'black';
    }
    $('#log').prepend('<span style="display: block; color: ' + color + ' ">' + s + '</span>');
}

function debug_disconnect() {
    loguear("Desconectando");
    window.game.connection.close();
    return false;
}


function debug_unsubscribe() {
    loguear("Desuscribiendo jugador");  
	window.game.unsubscribe("0");
    return false;
}

function debug_start() {
    loguear("Empezando el juego");  
    name = 'Start';
    data = [{"Id":1, "Name":"Carlos"}, {"Id":2, "Name":"Juan"}];
    do_debug(name, data)
    return false;
}

function debug_player() {
    loguear('Informacion usuario 1')
    name = 'Get_Info'
    data = {'t_id':"1", 'board':[[Codigo.Unknown, Codigo.Unknown],[Codigo.Unknown, Codigo.Unknown]]}
    do_debug(name, data)
    return false;
}

function do_debug(name, data) {
    window.game.connection.handle_message(JSON.stringify({'Name':name, 'Data':data}));
}

function encode_utf8(s) {
  return unescape(encodeURIComponent(s));
}

function decode_utf8(s) {
  return decodeURIComponent(escape(s));
}

function error(str) {
    $('#error').show()
    loguear('ERROR: ' + str, 'red');
    setTimeout(errorend(), 3000)
}

function errorend() {
     $('#error').hide()
}

function warning(str) {
    loguear('WARNING: ' + str, 'yellow');
}
