//Hacer el window.onload.
if(typeof $ == 'undefined' 
    || typeof Game == 'undefined' 
    || typeof Connection == 'undefined'
    || typeof Paneles == 'undefined') {
    alert('Faltan librerias')
};


$(document).ready(function() {
    mostrar_log();
    paneles = new Paneles();
    bindear_controles(paneles);
    
    
    paneles.coneccion.connect.click(function(e) {
        loguear("Conectando");
        var connection = new Connection('localhost', 5555);
        window.game = new Game(connection);
        paneles.coneccion.hide()
        paneles.registro.show()
        paneles.registro.nombre.focus()
        e.preventDefault();
    });
    
    paneles.registro.form.submit(function(e){
        var nombre = paneles.registro.nombre.val()
        window.game.subscribe(nombre);
        paneles.registro.hide()
        paneles.juego.show()
        paneles.juego.css('visibility','visible')
        return false;
    });
    paneles.coneccion.connect.focus();

    //TODO: Sacar
    paneles.juego.show()
});

function bindear_controles(paneles) {
    paneles.coneccion.connect = $('a#conectar , #conectar', paneles.coneccion);
    paneles.registro.form = $("#form-registro", paneles.registro);
    paneles.registro.nombre = $("#nombre_usuario", paneles.registro);
}

function do_wait_eta(eta) {
    window.game.do_wait_eta(eta);
}
