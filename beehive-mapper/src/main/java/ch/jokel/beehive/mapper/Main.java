package ch.jokel.beehive.mapper;

import ch.jokel.beehive.mapper.bean.TTNMessage;
import io.helidon.media.jsonb.server.JsonBindingSupport;
import io.helidon.webserver.Handler;
import io.helidon.webserver.Routing;
import io.helidon.webserver.WebServer;

public class Main {

    public static void main(String... args) {
        final JsonBindingSupport jsonBindingSupport = JsonBindingSupport.create();
        final Routing.Builder routingBuilder = Routing.builder();
        routingBuilder.register(jsonBindingSupport);
        Routing routing = routingBuilder
                .get("/greet", (req, res) -> res.send("Hello World!"))
                .post("/beedata", Handler.create(TTNMessage.class, (req, res, msg) -> res.send("Forwarded!")))
                .build();
        WebServer.create(routing).start();
    }
}
