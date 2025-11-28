const cars = [
    {
        make: "Tesla",
        model: "Model 3",
        price: "$42,000",
        image: "https://images.unsplash.com/photo-1560958089-b8a1929cea89?ixlib=rb-1.2.1&auto=format&fit=crop&w=500&q=60"
    },
    {
        make: "Porsche",
        model: "Taycan",
        price: "$85,000",
        image: "https://images.unsplash.com/photo-1617788138017-80ad40651399?ixlib=rb-1.2.1&auto=format&fit=crop&w=500&q=60"
    },
    {
        make: "Ford",
        model: "Mustang Mach-E",
        price: "$45,000",
        image: "https://images.unsplash.com/photo-1620882372658-5290b230238e?ixlib=rb-1.2.1&auto=format&fit=crop&w=500&q=60"
    }
];

const button = document.getElementById("my-btn");

button.addEventListener("click", function() {
    button.textContent = "Clicked!";
});

// 1. Select the toggle button
const themeToggle = document.getElementById('theme-toggle');

// 2. Listen for a click
themeToggle.addEventListener('click', () => {
    
    // 3. Toggle the class on the BODY element
    document.body.classList.toggle('dark-mode');
    
    // Optional: Switch the icon based on the mode
    if (document.body.classList.contains('dark-mode')) {
        themeToggle.textContent = "☀️"; // Change to Sun
    } else {
        themeToggle.textContent = "🌙"; // Change back to Moon
    }
});


// 3. Loop through the array and create HTML
const container = document.getElementById('project-container');

// Clear existing content (optional, but good practice)
container.innerHTML = "";

cars.forEach(car => {
    const cardHTML = `
        <div class="card">
            <img src="${car.image}" alt="${car.make}" style="width:100%; border-radius: 5px;">
            <h3>${car.make} ${car.model}</h3>
            <p>Starting at: <strong>${car.price}</strong></p>
            <button style="margin-top:10px; padding:5px 10px; cursor:pointer;">Buy Now</button>
        </div>
    `;
    container.innerHTML += cardHTML;
});