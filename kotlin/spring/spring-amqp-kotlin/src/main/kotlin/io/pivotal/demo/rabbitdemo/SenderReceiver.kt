package io.pivotal.demo.rabbitdemo

import org.springframework.amqp.core.Queue
import org.springframework.scheduling.annotation.Scheduled
import org.springframework.amqp.rabbit.core.RabbitTemplate
import org.springframework.amqp.rabbit.annotation.RabbitHandler
import org.springframework.amqp.rabbit.annotation.RabbitListener
import org.springframework.amqp.rabbit.connection.ConnectionFactory
import org.springframework.context.annotation.Bean
import org.springframework.context.annotation.Configuration
import org.springframework.context.annotation.Profile
import org.springframework.scheduling.annotation.EnableScheduling

class Sender(val template: RabbitTemplate, val queue: Queue)
{
    @Scheduled(fixedDelay = 1000, initialDelay = 500)
    fun send() {
        val message = "Hello World! from " + template.connectionFactory.host
        template.convertAndSend(queue.getName(), message)
        println("[x] Sent '$message'")
    }
}

@RabbitListener(queues = arrayOf("hello"))
class Receiver
{
    @RabbitHandler
    fun receive(messageIn: String) = println("[x] Received '$messageIn'")
}

//Create the necessary Beans based on the profile and turn on the scheduler
@Profile("!usage_message")
@Configuration
@EnableScheduling
class TutorialConfiguration
{
    @Bean
    fun hello() = Queue("hello")

    @Profile("receiver")
    @Bean
    fun receiver() = Receiver()

    @Profile("sender")
    @Bean
    fun sender(template: RabbitTemplate, queue: Queue) = Sender(template, queue)
}
