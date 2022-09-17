import java.io.*;
   
public class Salaries {
	
	private static float salarySum;
	private static int salaryCount;
	
	// Salaries class constructor and methods ----------------------------
	
	public Salaries()
	{
		// initializing salaries sum and count 
		// with some data already collected
		salarySum = 6000;
		salaryCount = 5;
	}
	
	public void addNewSalary(float newSalary)
	{
		salarySum += newSalary;
		salaryCount++;
	}
	
	public void substractAddedSalary(float addedSalary)
	{
		salarySum -= addedSalary;
		salaryCount--;
	}
	
	public void printStats()
	{
		System.out.println(
		    "Current average: " + (salarySum / salaryCount) +
		    " (sum: " + salarySum +
		    ", count: " + salaryCount + ")");
	}


	// static methods ----------------------------------------------------

	static float getSalary(BufferedReader input)
	// returns salary as provided by user, 
	// or 0 in case of any problems
	{
		float salary;
		
		try {
			salary = Float.parseFloat(input.readLine());
		} catch (NumberFormatException e) {
			System.out.println("Error: this is not a number");			
			salary = 0;
		} catch (IOException e) {
			System.out.println("Error: IO problem");			
			salary = 0;	
		}

		if (salary < 0 || Float.isInfinite(salary)) {
			System.out.println("Error: wrong value provided");			
			salary = 0;								
		}
		
		return salary;	
	}
	
	static boolean getYes(BufferedReader input)
	// returns true if user types "yes", otherwise returns false
	{
		try {
			return input.readLine().equalsIgnoreCase("yes");
		} catch (IOException e) {
			return false;
		}		
	}

	public static void main(String argv[])
	{
		Salaries salaries = new Salaries();
		float salary;
	
		// get a buffered input
		BufferedReader input = new BufferedReader(new InputStreamReader(System.in));
				
		salaries.printStats();
		do {
			System.out.println("---------------------------------------------\n" + 
			  "Please tell us your monthly salary in EUROs (type 0 to exit)");
			
			salary = getSalary(input);
			// if typed 0 then go to the end of the loop
			if (salary == 0) continue;
			
			salaries.addNewSalary(salary);
			salaries.printStats();
			
			System.out.println("Do you want the system to remember the salary you entered? (yes/no)");
			if (!getYes(input)) {
				// if answer is different than "yes"...
				System.out.println("... removing your last entry");
				salaries.substractAddedSalary(salary);
				salaries.printStats();
			} else
				System.out.println("... keeping your last entry");

		} while (salary > 0); 
	}
	
}
