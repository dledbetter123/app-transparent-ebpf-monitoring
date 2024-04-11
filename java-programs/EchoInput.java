import java.util.Scanner;

public class EchoInput {
    public static void main(String[] args) {
        Scanner scanner = new Scanner(System.in);
        
        while (true) {
            System.out.println("Please enter something (or type 'exit' to quit): ");
            String input = scanner.nextLine();
            
            if (input.equalsIgnoreCase("exit")) {
                break;
            }

            printInput(input);
        }

        System.out.println("Exiting program...");
    }

    public static void printInput(String input) {
        System.out.println("You entered: " + input);
    }
}
