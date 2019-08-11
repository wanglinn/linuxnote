测试可以看出：
使用方式1 变量synchronized方式，执行时间约1300ms, 使用阻塞队列方式约890ms, 使用方式3 双缓存序列执行时间约60ms。

//使用变量synchronized方式
import java.util.ArrayList;
import java.util.List;

 class Toy {
    private String name;

    public String getName() {
        return name;
    }
     public void setName(String name) {
        this.name = name;
    }
}



 class Factory extends Thread{


    public void run(){
        while(true){

        Toy t = new Toy ();

        t.setName("toy");
        synchronized (Tools.lT){
            Tools.lT.add(t);
         }
            try {
                Thread.sleep(2);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }

         }

}


 class Kid extends Thread {
    long time1 = System.currentTimeMillis();
    int count = 0;
    public void run() {
        while(true){

            synchronized(Tools.lT){
                if(Tools.lT.size()!=0)
            Tools.lT.remove(0);
            }
            count++;
            if(count==10000000){
                System.out.println("use time : " + (System.currentTimeMillis()-time1));
                System.exit(0);
            }

        }
        }
}


public class DoubleQueue {

    public static void main(String[] args) {
        Factory f = new Factory();
        f.start();
        Kid k = new Kid();
        k.start();

    }
}



 class Tools {
    public static List<Toy>lT = new ArrayList<Toy>(10000);
}



//使用阻塞队列方式
import java.util.ArrayList;
import java.util.List;

 class Toy {
    private String name;

    public String getName() {
        return name;
    }
public void setName(String name) {
        this.name = name;
    }
}



 class Factory extends Thread{


    public void run(){
        while(true){

        Toy t = new Toy ();

        t.setName("toy");
        //synchronized (Tools.lT){
            Tools.lT.add(t);
        // }
            try {
                Thread.sleep(2);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }

         }

}


 class Kid extends Thread {
    long time1 = System.currentTimeMillis();
    int count = 0;
    public void run() {
        while(true){

           // synchronized(Tools.lT){
                if(Tools.lT.size()!=0)
            Tools.lT.remove(0);
           // }
            count++;
            if(count==10000000){
                System.out.println("use time : " + (System.currentTimeMillis()-time1));
                System.exit(0);
            }

        }
        }
}


public class DoubleQueue1 {

    public static void main(String[] args) {
        Factory f = new Factory();
        f.start();
        Kid k = new Kid();
        k.start();

    }
}



 class Tools {
    public static LinkedBlockingQueue<Toy> lT= new LinkedBlockingQueue<Toy>(10000);
}


//双缓存队列方式
import java.util.ArrayList;
import java.util.List;

 class Toy {
    private String name;

    public String getName() {
        return name;
    }
public void setName(String name) {
        this.name = name;
    }
}



 class Factory extends Thread{


    public void run(){
        while(true){

        Toy t = new Toy ();

        t.setName("toy");
            Tools.lT.add(t);
            try {
                Thread.sleep(2);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }

         }

}


 class Kid extends Thread {
    long time1 = System.currentTimeMillis();
    int count = 0;
    public void run() {
        while(true){
                if(Tools.lT.size()!=0)
            Tools.lT.remove(0);
            count++;
            if(count==10000000){
                System.out.println("use time : " + (System.currentTimeMillis()-time1));
                System.exit(0);
            }

        }
        }
}


public class DoubleQueue2 {

    public static void main(String[] args) {
       Factory f = new Factory();
        f.start();
        Kid k = new Kid();
        k.start();
	   DoubleBufferList dbf = new DoubleBufferList(lT, lP,2);
	   dbf.check();

    }
}



 class Tools {
    public static List<Toy>lT = new ArrayList<Toy>(10000);
    public static List<Toy>lP = new ArrayList<Toy>(10000);
}


  class DoubleBufferList {

    private List<Object> lP;
    private List<Object> lT;
    private int gap;

    public DoubleBufferList(List lP, List lT, int gap) {
        this.lP = lP;
        this.lT = lT;
        this.gap = gap;
    }

    public void check() {
        Runnable runner = new Runnable() {
            public void run() {
                while (true) {
                    if (lT.size() == 0) {
                        synchronized (lT) {
                            synchronized (lP) {
                                lT.addAll(lP);
                            }
                            lP.clear();
                        }
                    }
                    try {
                        Thread.sleep(gap);

                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }
            }
        };
        Thread thread = new Thread(runner);
        thread.start();
    }

}



参考：https://blog.csdn.net/xcy13638760/article/details/40888973
