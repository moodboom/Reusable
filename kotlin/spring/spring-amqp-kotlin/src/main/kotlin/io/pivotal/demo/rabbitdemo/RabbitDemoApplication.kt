package io.pivotal.demo.rabbitdemo

import org.springframework.amqp.rabbit.connection.ConnectionFactory
import org.springframework.amqp.rabbit.core.RabbitTemplate
import org.springframework.beans.factory.annotation.Value
import org.springframework.boot.CommandLineRunner
import org.springframework.boot.SpringApplication
import org.springframework.boot.autoconfigure.SpringBootApplication
import org.springframework.context.ConfigurableApplicationContext
import org.springframework.context.annotation.Bean
import org.springframework.context.annotation.Profile

@SpringBootApplication
class RabbitDemoApplication
{
    //If the default profile is set, print the usage message and exit
    @Profile("usage_message")
    @Bean
    fun printUsage() = CommandLineRunner {
        println("This app uses Spring Profiles to control its behavior.");
        println("Sample usage:\n\njava -jar spring-amqp-kotlin.jar --spring.profiles.active=sender\n");
    }

    //If the default profile is not set, create the TutorialRunner to
    //run the app for a specific length of time
    @Profile("!usage_message")
    @Bean
    fun createTutorialRunner(ctx: ConfigurableApplicationContext) = TutorialRunner(ctx)
}

//Class to run the app for a duration and then shutdown
class TutorialRunner(val ctx: ConfigurableApplicationContext): CommandLineRunner
{
    override fun run(vararg p0: String?) {

        val cf = ctx.getBean(ConnectionFactory::class.java)
        println("Host: " + cf.host)
        println("Port: " + cf.port)
        println("User: " + cf.username)

        println("Ready ... running for ${duration}ms");
        Thread.sleep(duration);
        ctx.close();
    }

    @Value("\${client.duration:0}")
    val duration: Long = 10000
}

fun main(args: Array<String>) {
    SpringApplication.run(RabbitDemoApplication::class.java, *args)
}
