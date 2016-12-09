package com.bitpost.scala.scrap

// original brew silliness from:
// http://danielwestheide.com/blog/2013/01/09/the-neophytes-guide-to-scala-part-8-welcome-to-the-future.html

import scala.concurrent.ExecutionContext.Implicits.global

// MDM These are essential for futures and promises
import scala.concurrent.{Future, Promise}
import scala.util.{Failure, Success, Try}

// MDM SYNCHRONOUS BEHAVIOR, don't act like you never need it
import scala.concurrent.Await
import scala.concurrent.duration.Duration

import scala.util.Random


/**
  * Created by m on 9/30/16.
  */

object Scrap extends App {


  // 1 ===================================================================================
  // FUTURES and how to make them run async but then hit a point where they can sync up
  // 1 ===================================================================================
  println("== 1 ============================")
  println("== Making coffee")

  // Some type aliases, just for getting more meaningful method signatures:
  type CoffeeBeans = String
  type GroundCoffee = String
  case class Water(temperature: Int)
  type Milk = String
  type FrothedMilk = String
  type Espresso = String
  type Cappuccino = String

  def grind(beans: CoffeeBeans): Future[GroundCoffee] = Future {
    println("grind: start...")
    Thread.sleep(Random.nextInt(500))
    // if (beans == "baked beans") throw GrindingException("are you joking?")
    println("grind: finished...")
    s"ground coffee of $beans"
  }

  def heatWater(water: Water): Future[Water] = Future {
    println("heat :starting to heat the water now")
    Thread.sleep(Random.nextInt(3000))
    println("heat : hot, it's hot!")
    water.copy(temperature = 85)
  }

  def frothMilk(milk: Milk): Future[FrothedMilk] = Future {
    println("milk : frothing system engaged!")
    Thread.sleep(Random.nextInt(500))
    println("milk : shutting down frothing system")
    s"frothed $milk"
  }

  def brew(coffee: GroundCoffee, heatedWater: Water): Future[Espresso] = Future {
    println("brew : happy brewing :)")
    Thread.sleep(Random.nextInt(500))
    println("brew : it's brewed!")
    "espresso"
  }

  def combine(espresso: Espresso, frothedMilk: FrothedMilk): Cappuccino = s"${espresso}-${frothedMilk} cappuccino"

  def makeACupAndWaitForItToFinish() =
  {
    // Let's make some coffee...
    println("espresso: starting...")
    val groundCoffee = grind("arabica beans")
    val heatedWater = heatWater(Water(20))
    val frothedMilk = frothMilk("soymilk")
    val results = for {
      ground <- groundCoffee
      water <- heatedWater
      foam <- frothedMilk
      espresso <- brew(ground, water)
    } yield combine(espresso, foam)

    results onFailure { case t => println(s"wtf ${t.getMessage}") }
    results onSuccess {
      case actualFinalResults =>
        println(s"espresso: done with ${actualFinalResults}")
    }

    // WAIT for the damned coffee to finish before you quit, silly Scala!
    // Sometimes you NEED synchronous behavior, like when determining when to quit a command line app.
    // NOTE that in a typical server scenario, blocking is a sign of a bad design.
    Await.ready(results,Duration.Inf)
  }

  makeACupAndWaitForItToFinish()

  // Keep Scala somewhat honest
  // NOTE that in the goofy scala world, when this finishes
  // the final logging may happen after the next block starts.
  // Apparently, deterministic behavior is only available via true functional chaining.
  Thread.sleep(300)

  // 2 ===================================================================================
  // What do you use a promise for?  Not sure yet...
  // not for synchronous guarantee it seems - this doesn't block app to finish...
  // AH, a future IS a promise.  This just splits them.  What do you do with it then?
  // 2 ===================================================================================
  println("== 2 ============================")
  println("== I promise... ?")

  // This takes up a thread and doesn't let go
  // I'm turning it off so subsequent scrap is not impacted...
  /*
  def makeACoffeeViaPromise() = Future {
    println("instant: start to make a slow instant coffee")
    Thread.sleep(10000)
    println("instant: done.  gross.")
  }
  val p = Promise[Unit]
  val f = makeACoffeeViaPromise()
  f.onComplete {
    case Success(t) =>  p.success(t)
    case Failure(t) => p.failure(t)
  }
  p.future

  // Keep Scala somewhat honest
  Thread.sleep(300)
  */


  // 3 ===================================================================================
  // Container looping with futures
  // 3 ===================================================================================
  println("== 3 ============================")
  println("== Easy Await with Future[]")

  type Smoothie = String

  // Small list
  // val smoothies = List[Smoothie]( "mango", "blueberry", "strawberry", "banana" )

  // Big indexed "par" container
  // NOTE that it did not make much difference, it seems scala optimizes so that only VERY large collections are parallelized...?
  // val smoothies = List[Smoothie]( "mango", "blueberry", "strawberry", "mango", "blueberry", "strawberry", "mango", "blueberry", "strawberry", "mango", "blueberry", "strawberry", "mango", "blueberry", "strawberry", "mango", "blueberry", "strawberry", "mango", "blueberry", "strawberry", "mango", "blueberry", "strawberry", "mango", "blueberry", "strawberry", "mango", "blueberry", "strawberry", "mango", "blueberry", "strawberry", "mango", "blueberry", "strawberry", "mango", "blueberry", "strawberry", "mango", "blueberry", "strawberry", "mango", "blueberry", "strawberry", "mango", "blueberry", "strawberry", "mango", "blueberry", "strawberry", "mango", "blueberry", "strawberry", "mango", "blueberry", "strawberry", "mango", "blueberry", "strawberry", "mango", "blueberry", "strawberry", "mango", "blueberry", "strawberry", "mango", "blueberry", "strawberry", "mango", "blueberry", "strawberry", "banana" )
  //   .toIndexedSeq
  //   .par

  // Small indexed parallel-enable container
  val smoothies = List[Smoothie]( "mango", "blueberry", "strawberry", "banana" )
    .toIndexedSeq
    .par

  def makeSmoothie(smoothie: Smoothie) = Future {
    println(s"smoothie start: ${smoothie}")
    Thread.sleep(500)
    println(s"smoothie done : ${smoothie}")

    // A dirty little sleep here to get the logging out before something else jumps in!
    // Thread.sleep(50)
  }

  // 1) We can make a full set of smoothies fully async, if we want.  Fire and forget!
  // Just don't expect to know when it's complete.
  // NOTE that this will suck up threads and therefore may interfere with subsequent code.
  // That's the Scala life!
  // var smoothiesMapResult = smoothies.map(smoothie => makeSmoothie(smoothie))

  // 2) Filter and map the result as a parameter into a function, to only do ONE smoothie.
  // NOTE: This will not block
  // smoothies.filter(_ == "mango").map(smoothie => makeSmoothie(smoothie))

  // ---------------------------------------------------------
  // 3) Map wrapped with a future, SO WE KNOW WHEN IT IS DONE.
  // How nice that I completely stumbled on this.
  // WORKS GREAT
  var smoothiesFuture = Future{
    smoothies.map(smoothie => makeSmoothie(smoothie))
  }
  smoothiesFuture.onComplete {
    case Success(t) => println("Smoothies are all done.")
    case Failure(t) => println(s"Smoothies didn't get done: ${t}")
  }
  // Yes blocking is BAD BAD BAD, but you better do it
  // if you want to know when to exit your app.
  // Bah this doesn't actually block on the map internal operation.
  // It would take deeper blocking to get that done.
  // Not a good idea to spend effort on.
  Await.result(smoothiesFuture, Duration.Inf)
  // ---------------------------------------------------------

  // Keep Scala somewhat honest
  Thread.sleep(300)

  // 4 ===================================================================================
  // Need to handle future of futures completion
  // GOT IT
  // THINGS TO NOTE:
  //    1) know your types, and use these tools to flatten: val flatterFuture = ordersFuture.flatMap(s => Future.sequence(s))
  //    1) do not use .par to try to force parallel action, it will prevent flattening of Futures
  //    2) Await is NOT PRECISE at least with things like simultaneous println - always put a delay in!  Terrible.
  // 4 ===================================================================================
  println("== 4 ============================")
  println("== FLATTENING Future[Future[]]")

  type SmoothieOrder = String
  val orders = List[SmoothieOrder]( "jon", "cara", "stephen" )
    .toIndexedSeq
    // Don't use this, it doesn't flatten!!!
    //.par

  def prepOrder(order: SmoothieOrder) = Future {

    // Each order is a full smoothie set
    var smoothiesFuture = Future{
      smoothies.map(smoothie => makeSmoothie(smoothie))
    }
    smoothiesFuture
  }

  val ordersFuture = Future{
    orders.map(order => prepOrder(order))
  }

  // We have a future (outer) of list of future (inner) of list
  // How are we supposed to flatten this?
  val flattenedFutures = ordersFuture.flatMap(s => Future.sequence(s))
  // flattenedFutures.foreach(println)
  val flattenedFutures2 = flattenedFutures.flatMap(s => Future.sequence(s))
  // flattenedFutures2.foreach(println)
  Await.result(flattenedFutures2,Duration.Inf)

  flattenedFutures2.onComplete {
    case Success(t) => {

      // THIS IS MANDATORY, Scala is in a rush and will println this ahead of future println without it!
      Thread.sleep(1500)

      println("Orders are all done.")
    }
    case Failure(t) => println(s"Orders didn't get done: ${t}")
  }

  // Even though we tracked the end of a bunch of futures and used Await on them,
  // Scala doesn't seem to care and keeps going if we let it.
  Thread.sleep(5000)


  // 5 ===================================================================================
  // Convert a list into a map using the list as the key values
  // The trick is in the toMap call
  // WARNING this is NOT efficient
  // http://stackoverflow.com/questions/674639/scala-best-way-of-turning-a-collection-into-a-map-by-key
  // 5 ===================================================================================
  println("== 5 ============================")
  case class CConfig(ssdfhae:String,sffhae2:String,wh:Integer)
  val charms = List("a","b","c")
  val charmConfigs: Map[String, CConfig] = charms.map(charm => charm -> CConfig(charm,charm,10)) toMap

  // WOW, move this right under the above line and you'll get syntax errors wtf!
  println(s"""The a charm config = ${charmConfigs("a").sffhae2}""")


}
